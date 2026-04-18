/*
 * emotion_lamp.ino — Emotion Lamp v2.1
 * ESP32-WROOM-32E + INMP441 + WS2812B (16-LED Ring)
 *
 * Files:
 *   config.h  ← Edit settings here
 *   webui.h   ← Edit web UI here
 *   emotion_lamp.ino ← This file (all C++ logic)
 *
 * Pin wiring:
 *   INMP441 SCK→GPIO25 | WS→GPIO26 | SD→GPIO22 | L/R→GND | VDD→3.3V
 *   WS2812B DATA→GPIO4 (via SN74HC125N buffer)
 *
 * Libraries: FastLED, arduinoFFT (2.x)
 * WiFi.h is built into the ESP32 Arduino core.
 * Board: ESP32 Dev Module
 */

#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include <WiFi.h>
#include <FastLED.h>
#include <arduinoFFT.h>
#include "config.h"
#include "webui.h"

// ── Globals ───────────────────────────────────────────────────
CRGB    leds[NUM_LEDS];
CRGB    currentColor  = CRGB::White;
uint8_t currentBright = BRIGHT_QUIET;

double   vReal[SAMPLES];
double   vImag[SAMPLES];
int32_t  rawSamples[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

static i2s_chan_handle_t rx_chan = NULL;
WiFiServer server(80);

float smoothedRMS  = 0.0f;
float smoothedFreq = 200.0f;

volatile bool    audioReactiveEnabled = true;
// ★ User-controlled brightness cap (0-255). Slider sets this.
//   Defaults to BRIGHT_MAX from config.h.
volatile uint8_t userBrightness = BRIGHT_MAX;

// ── Section 1: I2S / Microphone ──────────────────────────────
void setupI2S() {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
      (i2s_port_t)I2S_PORT, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

  i2s_std_config_t std_cfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                    I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2S_SCK_PIN,
      .ws   = (gpio_num_t)I2S_WS_PIN,
      .dout = I2S_GPIO_UNUSED,
      .din  = (gpio_num_t)I2S_SD_PIN,
      .invert_flags = { .mclk_inv=false, .bclk_inv=false, .ws_inv=false },
    },
  };
  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}

float readAudio() {
  size_t bytesRead = 0;
  i2s_channel_read(rx_chan, rawSamples, sizeof(rawSamples), &bytesRead, portMAX_DELAY);
  double sum = 0.0;
  int count  = (int)(bytesRead / sizeof(int32_t));
  for (int i = 0; i < count; i++) {
    int32_t s = rawSamples[i] >> 14;
    vReal[i] = (double)s; vImag[i] = 0.0;
    sum += (double)s * s;
  }
  for (int i = count; i < SAMPLES; i++) { vReal[i] = 0.0; vImag[i] = 0.0; }
  return (count > 0) ? sqrtf((float)(sum / count)) : 0.0f;
}

double getDominantFreq() {
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  return FFT.majorPeak();
}

// ── Section 2: LED / Colour ───────────────────────────────────
// Colour anchor per frequency zone — edit CRGB values to change a zone's colour
static const CRGB COL_BLUE    = CRGB(  0, 150, 255);  // Bass          (calming light blue)
static const CRGB COL_PURPLE  = CRGB(100,   0, 255);  // Low speech    (electric purple)
static const CRGB COL_MAGENTA = CRGB(255,   0, 100);  // Mid speech    (vibrant magenta)
static const CRGB COL_ORANGE  = CRGB(255,  50,   0);  // Upper speech  (fiery orange-red)
static const CRGB COL_RED     = CRGB(139,   0,   0);  // Loud / shrill (intense dark red)

uint8_t lerpU8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)((float)a + t * ((float)b - (float)a));
}
CRGB lerpColor(CRGB a, CRGB b, float t) {
  return CRGB(lerpU8(a.r,b.r,t), lerpU8(a.g,b.g,t), lerpU8(a.b,b.b,t));
}

CRGB freqToColor(float freq) {
  if (freq < FREQ_BAND_1) return COL_BLUE;
  if (freq < FREQ_BAND_2) {
    float t=(freq-FREQ_BAND_1)/(float)(FREQ_BAND_2-FREQ_BAND_1);
    return lerpColor(COL_BLUE, COL_PURPLE, constrain(t,0.0f,1.0f));
  }
  if (freq < FREQ_BAND_3) {
    float t=(freq-FREQ_BAND_2)/(float)(FREQ_BAND_3-FREQ_BAND_2);
    return lerpColor(COL_PURPLE, COL_MAGENTA, constrain(t,0.0f,1.0f));
  }
  if (freq < FREQ_BAND_4) {
    float t=(freq-FREQ_BAND_3)/(float)(FREQ_BAND_4-FREQ_BAND_3);
    return lerpColor(COL_MAGENTA, COL_ORANGE, constrain(t,0.0f,1.0f));
  }
  return COL_RED;
}

void updateLamp(float rms, float freq) {
  CRGB    targetColor;
  uint8_t targetBright;

  if (rms < QUIET_THRESHOLD) {
    targetColor  = CRGB(255, 255, 255);
    targetBright = BRIGHT_QUIET;
  } else if (rms >= NOISY_THRESHOLD) {
    targetColor  = COL_RED;
    targetBright = BRIGHT_MAX;
  } else {
    targetColor  = freqToColor(freq);
    targetBright = (uint8_t)constrain(
      map((long)rms, QUIET_THRESHOLD, NOISY_THRESHOLD, BRIGHT_ACTIVE, BRIGHT_MAX),
      BRIGHT_ACTIVE, BRIGHT_MAX);
  }

  currentColor  = lerpColor(currentColor, targetColor, SMOOTH_FACTOR);
  currentBright = lerpU8(currentBright, targetBright, SMOOTH_FACTOR);

  // Apply userBrightness as a global master multiplier so the slider
  // affects ALL states (quiet, active, loud) proportionally.
  uint8_t finalBright = (uint8_t)((uint32_t)currentBright * userBrightness / 255);
  FastLED.setBrightness(finalBright);
  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();
}

// ── Section 3: WiFi AP + Web Server ──────────────────────────
void startWiFiAP() {
  IPAddress apIP   (AP_IP_1, AP_IP_2, AP_IP_3, AP_IP_4);
  IPAddress gateway(AP_IP_1, AP_IP_2, AP_IP_3, AP_IP_4);
  IPAddress subnet (255, 255, 255, 0);
  Serial.println("[WiFi] Starting Access Point...");
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("[WiFi] AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void startWebServer() {
  server.begin();
  Serial.printf("[Web]  Server at http://%d.%d.%d.%d\n",
    AP_IP_1, AP_IP_2, AP_IP_3, AP_IP_4);
}

// Sends JSON with current lamp state — fetched by the webpage on load
// and periodically for live RMS/Freq display.
void sendStatus(WiFiClient& client) {
  char json[900];
  int n = snprintf(json, sizeof(json),
    "{\"on\":%s,\"brightness\":%d,\"rms\":%d,\"freq\":%d,"
    "\"zones\":["
    "{\"name\":\"Quiet Room\",\"desc\":\"No sound detected (RMS < %d)\",\"hex\":\"#ffffff\",\"label\":\"(255, 255, 255)\"},"
    "{\"name\":\"Bass\",\"desc\":\"Deep bass or rumble (below %d Hz)\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d, %d, %d)\"},"
    "{\"name\":\"Low Speech\",\"desc\":\"Low voice or hum (%d to %d Hz)\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d, %d, %d)\"},"
    "{\"name\":\"Mid Speech\",\"desc\":\"Normal conversation (%d to %d Hz)\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d, %d, %d)\"},"
    "{\"name\":\"Upper Speech\",\"desc\":\"Raised voice or high pitch (%d to %d Hz)\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d, %d, %d)\"},"
    "{\"name\":\"Loud / Shrill\",\"desc\":\"Loud override (RMS > %d or freq > %d Hz)\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d, %d, %d)\"}"
    "]}",
    audioReactiveEnabled ? "true" : "false",
    (int)userBrightness,
    (int)smoothedRMS, (int)smoothedFreq,
    QUIET_THRESHOLD,
    FREQ_BAND_1,
      (int)COL_BLUE.r,    (int)COL_BLUE.g,    (int)COL_BLUE.b,
      (int)COL_BLUE.r,    (int)COL_BLUE.g,    (int)COL_BLUE.b,
    FREQ_BAND_1, FREQ_BAND_2,
      (int)COL_PURPLE.r,  (int)COL_PURPLE.g,  (int)COL_PURPLE.b,
      (int)COL_PURPLE.r,  (int)COL_PURPLE.g,  (int)COL_PURPLE.b,
    FREQ_BAND_2, FREQ_BAND_3,
      (int)COL_MAGENTA.r, (int)COL_MAGENTA.g, (int)COL_MAGENTA.b,
      (int)COL_MAGENTA.r, (int)COL_MAGENTA.g, (int)COL_MAGENTA.b,
    FREQ_BAND_3, FREQ_BAND_4,
      (int)COL_ORANGE.r,  (int)COL_ORANGE.g,  (int)COL_ORANGE.b,
      (int)COL_ORANGE.r,  (int)COL_ORANGE.g,  (int)COL_ORANGE.b,
    NOISY_THRESHOLD, FREQ_BAND_4,           // swapped to match new desc order
      (int)COL_RED.r,     (int)COL_RED.g,     (int)COL_RED.b,
      (int)COL_RED.r,     (int)COL_RED.g,     (int)COL_RED.b
  );
  client.printf(
    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\nContent-Length: %d\r\n"
    "Connection: close\r\n\r\n", n);
  client.write((uint8_t*)json, n);
}

// Main HTTP request handler — called every loop()
void processWebClients() {
  WiFiClient client = server.available();
  if (!client) return;

  unsigned long t = millis();
  while (!client.available() && millis() - t < 200) delay(1);

  String req = client.readStringUntil('\r');
  client.flush();

  // ── Route: /status ─────────────────────────────────────────
  if (req.indexOf("/status") != -1) {
    sendStatus(client);

  // ── Route: /brightness?val=N ───────────────────────────────
  } else if (req.indexOf("/brightness") != -1) {
    int vi = req.indexOf("val=");
    if (vi != -1) {
      String vs = req.substring(vi + 4);
      int sp = vs.indexOf(' '); if (sp != -1) vs = vs.substring(0, sp);
      userBrightness = (uint8_t)constrain(vs.toInt(), 0, 255);
      Serial.printf("[Web] Brightness -> %d\n", (int)userBrightness);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /off ────────────────────────────────────────────
  } else if (req.indexOf("/off") != -1) {
    audioReactiveEnabled = false;
    Serial.println("[Web] Lamp OFF");
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOFF");

  // ── Route: /on ─────────────────────────────────────────────
  } else if (req.indexOf("/on") != -1) {
    audioReactiveEnabled = true;
    Serial.println("[Web] Audio reactive ON");
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nON");

  // ── Route: / (main page) ───────────────────────────────────
  } else {
    uint16_t len = strlen_P(INDEX_HTML);
    client.printf(
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
      "Content-Length: %u\r\nConnection: close\r\n\r\n", len);
    char buf[256]; uint16_t pos = 0;
    while (pos < len) {
      uint16_t chunk = min((uint16_t)256, (uint16_t)(len - pos));
      memcpy_P(buf, INDEX_HTML + pos, chunk);
      client.write((uint8_t*)buf, chunk);
      pos += chunk;
    }
  }
  delay(1);
  client.stop();
}

// ── setup() and loop() ────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHT_QUIET);
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  startWiFiAP();
  startWebServer();
  setupI2S();
  Serial.println("[Lamp] Emotion Lamp v2.1 ready.");
}

void loop() {
  // ── Lamp OFF: fade LEDs to black over ~1 second ─────────────
  if (!audioReactiveEnabled) {
    if (currentBright > 0) {
      // Subtract 8 per frame (30ms) → full fade in ~1s
      currentBright = (currentBright > 8) ? currentBright - 8 : 0;
      uint8_t fb = (uint8_t)((uint32_t)currentBright * userBrightness / 255);
      FastLED.setBrightness(fb);
      FastLED.show();
    }
    processWebClients();
    delay(30);
    return;
  }

  // ── Audio Reactive ───────────────────────────────────────────
  // When turning back ON, currentBright is ~0 so lerpU8 in
  // updateLamp() naturally fades the brightness back in.
  float  rawRMS  = readAudio();
  double rawFreq = getDominantFreq();
  smoothedRMS  += SMOOTH_FACTOR * (rawRMS         - smoothedRMS);
  smoothedFreq += SMOOTH_FACTOR * ((float)rawFreq - smoothedFreq);
  Serial.printf("RMS: %4d  Freq: %4d Hz\n", (int)smoothedRMS, (int)smoothedFreq);
  updateLamp(smoothedRMS, smoothedFreq);
  processWebClients();
  delay(30);
}

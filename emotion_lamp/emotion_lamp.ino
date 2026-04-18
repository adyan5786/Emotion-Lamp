/*
 * ============================================================
 *  emotion_lamp.ino — Emotion Lamp v2.0
 *  ESP32-WROOM-32E + INMP441 + WS2812B (16-LED Ring)
 * ============================================================
 *
 *  Behavior:
 *    - Quiet room  → Dim white
 *    - Active sound → Blue → Cyan → Purple → Orange → Red
 *                     (colour tracks dominant frequency)
 *    - Very loud   → Full red
 *    - Web control → http://4.3.2.1  (connect to "Emotion-Lamp" AP)
 *
 *  Files:
 *    config.h          ← ★ Edit this to change settings
 *    webui.h           ← Edit this to change the web page look
 *    emotion_lamp.ino  ← This file (all C++ logic)
 *
 *  Pin wiring (ESP32-WROOM-32E):
 *    INMP441  SCK → GPIO25  |  WS → GPIO26  |  SD → GPIO22
 *    INMP441  L/R → GND    |  VDD → 3.3V   |  GND → GND
 *    WS2812B DATA → GPIO4  (via SN74HC125N buffer)
 *
 *  Libraries needed (Arduino Library Manager):
 *    - FastLED      by Daniel Garcia
 *    - arduinoFFT   by Enrique Condes  (version 2.x)
 *    WiFi.h and WebServer.h are built into the ESP32 Arduino core.
 *
 *  Board: Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
 * ============================================================
 */

// ── Includes ──────────────────────────────────────────────────
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include <WiFi.h>       // WiFiServer + WiFiClient — built into ESP32 core
#include <FastLED.h>
#include <arduinoFFT.h>
#include "config.h"   // All tunable settings
#include "webui.h"    // INDEX_HTML web page string

// ─────────────────────────────────────────────────────────────
//  Globals
// ─────────────────────────────────────────────────────────────
CRGB    leds[NUM_LEDS];
CRGB    currentColor  = CRGB::White;
uint8_t currentBright = BRIGHT_QUIET;

double   vReal[SAMPLES];
double   vImag[SAMPLES];
int32_t  rawSamples[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

static i2s_chan_handle_t rx_chan = NULL;

WiFiServer server(80);  // Raw TCP server on port 80 — works on all ESP32 core versions

float smoothedRMS  = 0.0f;
float smoothedFreq = 200.0f;

// true = audio reactive, false = lamp off
// volatile so web server callback and loop() share it safely
volatile bool audioReactiveEnabled = true;

// ─────────────────────────────────────────────────────────────
//  Section 1 — I2S / Microphone
// ─────────────────────────────────────────────────────────────
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
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv   = false,
      },
    },
  };
  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT; // L/R=GND → left channel

  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}

float readAudio() {
  size_t bytesRead = 0;
  i2s_channel_read(rx_chan, rawSamples, sizeof(rawSamples),
                   &bytesRead, portMAX_DELAY);
  double sum   = 0.0;
  int    count = (int)(bytesRead / sizeof(int32_t));
  for (int i = 0; i < count; i++) {
    int32_t s = rawSamples[i] >> 14;  // INMP441: 18-bit left-justified
    vReal[i]  = (double)s;
    vImag[i]  = 0.0;
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

// ─────────────────────────────────────────────────────────────
//  Section 2 — LED / Colour Logic
// ─────────────────────────────────────────────────────────────

// Anchor colours for each frequency zone (edit CRGB values to change a zone)
static const CRGB COL_BLUE   = CRGB( 10,  30, 255);  // Zone 1 — deep bass
static const CRGB COL_CYAN   = CRGB(  0, 220, 200);  // Zone 2 — low speech
static const CRGB COL_PURPLE = CRGB(160,  20, 220);  // Zone 3 — mid speech
static const CRGB COL_ORANGE = CRGB(255, 100,   0);  // Zone 4 — upper speech
static const CRGB COL_RED    = CRGB(255,  15,  15);  // Zone 5 — shrill/loud

uint8_t lerpU8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)((float)a + t * ((float)b - (float)a));
}

CRGB lerpColor(CRGB a, CRGB b, float t) {
  return CRGB(lerpU8(a.r, b.r, t),
              lerpU8(a.g, b.g, t),
              lerpU8(a.b, b.b, t));
}

CRGB freqToColor(float freq) {
  if (freq < FREQ_BAND_1) {
    return COL_BLUE;
  } else if (freq < FREQ_BAND_2) {
    float t = (freq - FREQ_BAND_1) / (float)(FREQ_BAND_2 - FREQ_BAND_1);
    return lerpColor(COL_BLUE, COL_CYAN, constrain(t, 0.0f, 1.0f));
  } else if (freq < FREQ_BAND_3) {
    float t = (freq - FREQ_BAND_2) / (float)(FREQ_BAND_3 - FREQ_BAND_2);
    return lerpColor(COL_CYAN, COL_PURPLE, constrain(t, 0.0f, 1.0f));
  } else if (freq < FREQ_BAND_4) {
    float t = (freq - FREQ_BAND_3) / (float)(FREQ_BAND_4 - FREQ_BAND_3);
    return lerpColor(COL_PURPLE, COL_ORANGE, constrain(t, 0.0f, 1.0f));
  } else {
    return COL_RED;
  }
}

void updateLamp(float rms, float freq) {
  CRGB    targetColor;
  uint8_t targetBright;

  if (rms < QUIET_THRESHOLD) {
    targetColor  = CRGB(255, 255, 255);   // Quiet → dim white
    targetBright = BRIGHT_QUIET;

  } else if (rms >= NOISY_THRESHOLD) {
    targetColor  = COL_RED;               // Very loud → full red
    targetBright = BRIGHT_MAX;

  } else {
    targetColor  = freqToColor(freq);     // Active → colour from pitch
    targetBright = (uint8_t)constrain(
      map((long)rms, QUIET_THRESHOLD, NOISY_THRESHOLD, BRIGHT_ACTIVE, BRIGHT_MAX),
      BRIGHT_ACTIVE, BRIGHT_MAX);
  }

  currentColor  = lerpColor(currentColor, targetColor, SMOOTH_FACTOR);
  currentBright = lerpU8(currentBright, targetBright, SMOOTH_FACTOR);

  FastLED.setBrightness(currentBright);
  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();
}

// ─────────────────────────────────────────────────────────────
//  Section 3 — WiFi AP + Web Server
//  Uses WiFiServer/WiFiClient (raw TCP) — built into WiFi.h,
//  compatible with every ESP32 Arduino core version.
// ─────────────────────────────────────────────────────────────
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

// Call this every loop() to process one pending HTTP request.
// Reads the first line ("GET /path HTTP/1.1"), serves the right
// response, then closes the connection. Takes <1 ms when idle.
void processWebClients() {
  WiFiClient client = server.available();
  if (!client) return;

  // Wait briefly for data
  unsigned long t = millis();
  while (!client.available() && millis() - t < 200) delay(1);

  // Read the request line (e.g. "GET /off HTTP/1.1")
  String req = client.readStringUntil('\r');
  client.flush();

  // Route: /off
  if (req.indexOf("/off") != -1) {
    audioReactiveEnabled = false;
    Serial.println("[Web] Lamp turned OFF");
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOFF");

  // Route: /on
  } else if (req.indexOf("/on") != -1) {
    audioReactiveEnabled = true;
    Serial.println("[Web] Audio reactive ON");
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nON");

  // Route: / (control page)
  } else {
    // Send INDEX_HTML from PROGMEM in one chunk
    uint16_t len = strlen_P(INDEX_HTML);
    client.printf("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %u\r\nConnection: close\r\n\r\n", len);
    // Stream PROGMEM string in 256-byte chunks to avoid stack overflow
    char buf[256];
    uint16_t pos = 0;
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

// ─────────────────────────────────────────────────────────────
//  setup() and loop()
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHT_QUIET);
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();

  startWiFiAP();
  startWebServer();
  setupI2S();

  Serial.println("[Lamp] Emotion Lamp v2.0 ready.");
}

void loop() {
  // ── Lamp OFF ──────────────────────────────────────────────
  if (!audioReactiveEnabled) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    processWebClients();
    delay(50);
    return;
  }

  // ── Audio Reactive ────────────────────────────────────────
  float  rawRMS  = readAudio();
  double rawFreq = getDominantFreq();

  smoothedRMS  += SMOOTH_FACTOR * (rawRMS         - smoothedRMS);
  smoothedFreq += SMOOTH_FACTOR * ((float)rawFreq - smoothedFreq);

  Serial.printf("RMS: %4d  Freq: %4d Hz\n",
    (int)smoothedRMS, (int)smoothedFreq);

  updateLamp(smoothedRMS, smoothedFreq);

  processWebClients();  // Handle any pending HTTP request (~0 ms when idle)
  delay(30);
}

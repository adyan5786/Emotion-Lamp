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
#include <Preferences.h>   // ESP32 NVS — persists across power cycles
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

volatile bool    audioReactiveEnabled = false; // always start in standby
volatile uint8_t userBrightness = BRIGHT_MAX; // overwritten by NVS on boot

// ── Audio Tuning Variables (Loaded from NVS, defaults from config.h) ──
int quietThresh = QUIET_THRESHOLD;
int noisyThresh = NOISY_THRESHOLD;
int freqBands[4] = { FREQ_BAND_1, FREQ_BAND_2, FREQ_BAND_3, FREQ_BAND_4 };

// ── Persistent settings types ─────────────────────────────────
enum AnimStyle { ANIM_SOLID = 0, ANIM_MATRIX = 1 };
AnimStyle activeAnim = ANIM_SOLID;
unsigned long lastMatrixUpdate = 0;
#define MATRIX_SPEED_MS 40

enum LampMode : uint8_t {
  MODE_DEFAULT=0,
  MODE_CUSTOM1=1, MODE_CUSTOM2=2, MODE_CUSTOM3=3,
  MODE_CUSTOM4=4, MODE_CUSTOM5=5
};

// One colour profile = 5 zone colours + a name for future multi-profile support
struct ZoneColor  { uint8_t r, g, b; };
struct ColorProfile {
  ZoneColor zones[5];
  char      name[12];  // "Custom", "Night", etc. — ready for Phase 3B
};

// Factory defaults — mirrors the COL_ constants below
// ★ Edit these to change what "Default" mode looks like
static const ZoneColor DEFAULT_ZONES[5] = {
  {  0, 150, 255},  // Zone 0: Bass
  {100,   0, 255},  // Zone 1: Low Speech
  {255,   0, 100},  // Zone 2: Mid Speech
  {255,  50,   0},  // Zone 3: Upper Speech
  {139,   0,   0},  // Zone 4: Loud / Shrill
};

volatile LampMode activeMode = MODE_DEFAULT;  // overwritten by NVS on boot
ColorProfile      customProfiles[5];           // 5 custom profiles (Custom 1-5)
CRGB              activeColors[5];             // what freqToColor() reads
Preferences       prefs;                       // NVS handle

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
// Static constants kept for reference; freqToColor() uses activeColors[].
static const CRGB COL_BLUE    = CRGB(  0, 150, 255);
static const CRGB COL_PURPLE  = CRGB(100,   0, 255);
static const CRGB COL_MAGENTA = CRGB(255,   0, 100);
static const CRGB COL_ORANGE  = CRGB(255,  50,   0);
static const CRGB COL_RED     = CRGB(139,   0,   0);

// Copy correct zone colours into activeColors[] based on current mode.
void applyActiveColors() {
  if (activeMode == MODE_DEFAULT) {
    activeColors[0] = COL_BLUE;    activeColors[1] = COL_PURPLE;
    activeColors[2] = COL_MAGENTA; activeColors[3] = COL_ORANGE;
    activeColors[4] = COL_RED;
  } else {
    int pi = (int)activeMode - 1;           // 1..5 → 0..4
    pi = constrain(pi, 0, 4);
    for (int i = 0; i < 5; i++)
      activeColors[i] = CRGB(customProfiles[pi].zones[i].r,
                             customProfiles[pi].zones[i].g,
                             customProfiles[pi].zones[i].b);
  }
}

// ── NVS load / save ───────────────────────────────────────────
void loadPrefs() {
  prefs.begin("elamp", true);
  userBrightness = prefs.getUChar("brightness", BRIGHT_MAX);
  activeMode     = (LampMode)constrain(prefs.getUChar("mode", 0), 0, 5);
  activeAnim     = (AnimStyle)constrain(prefs.getUChar("anim", 0), 0, 1);
  quietThresh    = prefs.getInt("quiet", QUIET_THRESHOLD);
  noisyThresh    = prefs.getInt("noisy", NOISY_THRESHOLD);
  freqBands[0]   = prefs.getInt("fb0", FREQ_BAND_1);
  freqBands[1]   = prefs.getInt("fb1", FREQ_BAND_2);
  freqBands[2]   = prefs.getInt("fb2", FREQ_BAND_3);
  freqBands[3]   = prefs.getInt("fb3", FREQ_BAND_4);
  for (int p = 0; p < 5; p++) {
    char key[10]; sprintf(key, "profile%d", p);
    size_t got = prefs.getBytes(key, &customProfiles[p], sizeof(customProfiles[p]));
    if (got != sizeof(customProfiles[p])) {
      for (int z = 0; z < 5; z++) customProfiles[p].zones[z] = DEFAULT_ZONES[z];
      char nm[12]; sprintf(nm, "Custom %d", p+1);
      strncpy(customProfiles[p].name, nm, sizeof(customProfiles[p].name));
    }
  }
  prefs.end();
  Serial.println("[NVS] Prefs loaded.");
}

void savePrefs() {
  prefs.begin("elamp", false);
  prefs.putUChar("brightness", (uint8_t)userBrightness);
  prefs.putUChar("mode",       (uint8_t)activeMode);
  prefs.putUChar("anim",       (uint8_t)activeAnim);
  prefs.putInt("quiet",        quietThresh);
  prefs.putInt("noisy",        noisyThresh);
  prefs.putInt("fb0",          freqBands[0]);
  prefs.putInt("fb1",          freqBands[1]);
  prefs.putInt("fb2",          freqBands[2]);
  prefs.putInt("fb3",          freqBands[3]);
  for (int p = 0; p < 5; p++) {
    char key[10]; sprintf(key, "profile%d", p);
    prefs.putBytes(key, &customProfiles[p], sizeof(customProfiles[p]));
  }
  prefs.end();
  Serial.println("[NVS] Prefs saved.");
}

uint8_t lerpU8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)((float)a + t * ((float)b - (float)a));
}
CRGB lerpColor(CRGB a, CRGB b, float t) {
  return CRGB(lerpU8(a.r,b.r,t), lerpU8(a.g,b.g,t), lerpU8(a.b,b.b,t));
}

CRGB freqToColor(float freq) {
  // Uses activeColors[] so Default and Custom modes both work
  if (freq < freqBands[0]) return activeColors[0];
  if (freq < freqBands[1]) {
    float t=(freq-freqBands[0])/(float)(freqBands[1]-freqBands[0]);
    return lerpColor(activeColors[0], activeColors[1], constrain(t,0.0f,1.0f));
  }
  if (freq < freqBands[2]) {
    float t=(freq-freqBands[1])/(float)(freqBands[2]-freqBands[1]);
    return lerpColor(activeColors[1], activeColors[2], constrain(t,0.0f,1.0f));
  }
  if (freq < freqBands[3]) {
    float t=(freq-freqBands[2])/(float)(freqBands[3]-freqBands[2]);
    return lerpColor(activeColors[2], activeColors[3], constrain(t,0.0f,1.0f));
  }
  return activeColors[4];
}

void updateLamp(float rms, float freq) {
  CRGB    targetColor;
  uint8_t targetBright;

  if (rms < quietThresh) {
    targetColor  = CRGB(255, 255, 255);
    targetBright = BRIGHT_QUIET;
  } else if (rms >= noisyThresh) {
    targetColor  = activeColors[4];  // loud override colour from active profile
    targetBright = BRIGHT_MAX;
  } else {
    targetColor  = freqToColor(freq);
    targetBright = (uint8_t)constrain(
      map((long)rms, quietThresh, noisyThresh, BRIGHT_ACTIVE, BRIGHT_MAX),
      BRIGHT_ACTIVE, BRIGHT_MAX);
  }

  currentColor  = lerpColor(currentColor, targetColor, SMOOTH_FACTOR);
  currentBright = lerpU8(currentBright, targetBright, SMOOTH_FACTOR);

  // Apply userBrightness as a global master multiplier so the slider
  // affects ALL states (quiet, active, loud) proportionally.
  uint8_t finalBright = (uint8_t)((uint32_t)currentBright * userBrightness / 255);
  FastLED.setBrightness(finalBright);

  if (activeAnim == ANIM_SOLID) {
    fill_solid(leds, NUM_LEDS, currentColor);
  } else {
    // ANIM_MATRIX
    if (millis() - lastMatrixUpdate > MATRIX_SPEED_MS) {
      lastMatrixUpdate = millis();
      // Shift all LEDs one step to the right (up the strip)
      for (int i = NUM_LEDS - 1; i > 0; i--) {
        leds[i] = leds[i - 1];
      }
      // Insert the newest colour at the beginning
      leds[0] = currentColor;
    }
  }

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
  char json[1600];
  int  pos = 0;
  // Header
  pos += snprintf(json+pos, sizeof(json)-pos,
    "{\"on\":%s,\"brightness\":%d,\"anim\":%d,\"mode\":%d,"
    "\"audio\":{\"q\":%d,\"n\":%d,\"f\":[%d,%d,%d,%d]},",
    audioReactiveEnabled?"true":"false", (int)userBrightness, (int)activeAnim, (int)activeMode,
    quietThresh, noisyThresh, freqBands[0], freqBands[1], freqBands[2], freqBands[3]);
  // Profile names
  pos += snprintf(json+pos, sizeof(json)-pos, "\"profileNames\":[");
  for (int p = 0; p < 5; p++)
    pos += snprintf(json+pos, sizeof(json)-pos, "\"%s\"%s",
      customProfiles[p].name, p<4?",":"");
  pos += snprintf(json+pos, sizeof(json)-pos, "],");
  // All 5 profiles colours
  pos += snprintf(json+pos, sizeof(json)-pos, "\"allProfiles\":[");
  for (int p = 0; p < 5; p++) {
    pos += snprintf(json+pos, sizeof(json)-pos, "[");
    for (int z = 0; z < 5; z++)
      pos += snprintf(json+pos, sizeof(json)-pos, "[%d,%d,%d]%s",
        (int)customProfiles[p].zones[z].r,
        (int)customProfiles[p].zones[z].g,
        (int)customProfiles[p].zones[z].b, z<4?",":"");
    pos += snprintf(json+pos, sizeof(json)-pos, "]%s", p<4?",":"");
  }
  pos += snprintf(json+pos, sizeof(json)-pos, "],");
  // Active zones (from activeColors[])
  pos += snprintf(json+pos, sizeof(json)-pos,
    "\"zones\":["
    "{\"name\":\"Quiet Room\",\"desc\":\"Silence or near-silence\",\"hex\":\"#ffffff\",\"label\":\"(255,255,255)\"},"
    "{\"name\":\"Bass\",\"desc\":\"Deep bass and low rumbles\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"},"
    "{\"name\":\"Low Speech\",\"desc\":\"Low voices and soft hums\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"},"
    "{\"name\":\"Mid Speech\",\"desc\":\"Normal conversation\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"},"
    "{\"name\":\"Upper Speech\",\"desc\":\"Raised voices and high tones\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"},"
    "{\"name\":\"Loud/Shrill\",\"desc\":\"Loud or shrill sounds\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"}"
    "]}",
      (int)activeColors[0].r,(int)activeColors[0].g,(int)activeColors[0].b,
      (int)activeColors[0].r,(int)activeColors[0].g,(int)activeColors[0].b,
      (int)activeColors[1].r,(int)activeColors[1].g,(int)activeColors[1].b,
      (int)activeColors[1].r,(int)activeColors[1].g,(int)activeColors[1].b,
      (int)activeColors[2].r,(int)activeColors[2].g,(int)activeColors[2].b,
      (int)activeColors[2].r,(int)activeColors[2].g,(int)activeColors[2].b,
      (int)activeColors[3].r,(int)activeColors[3].g,(int)activeColors[3].b,
      (int)activeColors[3].r,(int)activeColors[3].g,(int)activeColors[3].b,
      (int)activeColors[4].r,(int)activeColors[4].g,(int)activeColors[4].b,
      (int)activeColors[4].r,(int)activeColors[4].g,(int)activeColors[4].b);
  client.printf(
    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\nContent-Length: %d\r\n"
    "Connection: close\r\n\r\n", pos);
  client.write((uint8_t*)json, pos);
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
      savePrefs();  // persist to NVS
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");
  // ── Route: /setzone?idx=N&r=R&g=G&b=B — stages colour in active profile
  } else if (req.indexOf("/setzone") != -1) {
    int ii=req.indexOf("idx="),ri=req.indexOf("&r="),gi=req.indexOf("&g="),bi=req.indexOf("&b=");
    if (ii!=-1&&ri!=-1&&gi!=-1&&bi!=-1&&activeMode!=MODE_DEFAULT) {
      int pi =constrain((int)activeMode-1,0,4);
      int idx=constrain(req.substring(ii+4).toInt(),0,4);
      int rv =constrain(req.substring(ri+3).toInt(),0,255);
      int gv =constrain(req.substring(gi+3).toInt(),0,255);
      int bv =constrain(req.substring(bi+3).toInt(),0,255);
      customProfiles[pi].zones[idx]={(uint8_t)rv,(uint8_t)gv,(uint8_t)bv};
      Serial.printf("[Web] Profile %d Zone %d staged->(%d,%d,%d)\n",pi,idx,rv,gv,bv);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /setmode?m=N (0=default, 1-5=custom)
  } else if (req.indexOf("/setmode") != -1) {
    int mi=req.indexOf("m=");
    if (mi!=-1) {
      activeMode=(LampMode)constrain(req.substring(mi+2).toInt(),0,5);
      applyActiveColors();
      savePrefs();
      Serial.printf("[Web] Mode->%d\n",(int)activeMode);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /setanim?a=N (0=solid, 1=matrix)
  } else if (req.indexOf("/setanim") != -1) {
    int ai=req.indexOf("a=");
    if (ai!=-1) {
      activeAnim=(AnimStyle)constrain(req.substring(ai+2).toInt(),0,1);
      savePrefs();
      Serial.printf("[Web] Anim->%d\n",(int)activeAnim);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /setname?p=N&name=... — update a profile's display name
  } else if (req.indexOf("/setname") != -1) {
    int pi=req.indexOf("p="), ni=req.indexOf("name=");
    if (pi!=-1&&ni!=-1) {
      int p=constrain(req.substring(pi+2).toInt(),0,4);
      String nm=req.substring(ni+5);
      int sp=nm.indexOf(' ');if(sp!=-1)nm=nm.substring(0,sp);
      nm.replace("+"," "); nm.replace("%20"," ");
      if(nm.length()>11)nm=nm.substring(0,11);
      strncpy(customProfiles[p].name,nm.c_str(),sizeof(customProfiles[p].name)-1);
      customProfiles[p].name[sizeof(customProfiles[p].name)-1]='\0';
      Serial.printf("[Web] Profile %d name->%s\n",p,customProfiles[p].name);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /saveprofile — persist all profiles to NVS
  } else if (req.indexOf("/saveprofile") != -1) {
    applyActiveColors();
    savePrefs();
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /discardprofile?p=N — reload profile from NVS, undo staged RAM edits
  } else if (req.indexOf("/discardprofile") != -1) {
    int di=req.indexOf("p=");
    if (di!=-1) {
      int p=constrain(req.substring(di+2).toInt(),0,4);
      char key[10]; sprintf(key,"profile%d",p);
      prefs.begin("elamp",true);
      size_t got=prefs.getBytes(key,&customProfiles[p],sizeof(customProfiles[p]));
      prefs.end();
      if (got!=sizeof(customProfiles[p])) {
        for(int z=0;z<5;z++) customProfiles[p].zones[z]=DEFAULT_ZONES[z];
        char nm[12]; sprintf(nm,"Custom %d",p+1);
        strncpy(customProfiles[p].name,nm,sizeof(customProfiles[p].name));
      }
      if((int)activeMode==p+1) applyActiveColors(); // re-apply if this is active profile
      Serial.printf("[Web] Profile %d discarded (reverted from NVS)\n",p);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /reboot ───────────────────────────────────────────
  } else if (req.indexOf("/reboot") != -1) {
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");
    client.flush(); delay(100); client.stop(); delay(400);
    ESP.restart();

  // ── Route: /reset ────────────────────────────────────────────
  } else if (req.indexOf("/reset") != -1) {
    prefs.begin("elamp", false);
    prefs.clear();
    prefs.end();
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");
    client.flush(); delay(100); client.stop(); delay(400);
    ESP.restart();

  // ── Route: /setaudio?q=..&n=..&f0=..&f1=..&f2=..&f3=.. ──────
  } else if (req.indexOf("/setaudio") != -1) {
    auto getInt = [](String r, String k, int def) {
      int i = r.indexOf(k + "=");
      if (i == -1) return def;
      int e = r.indexOf('&', i);
      if (e == -1) e = r.indexOf(' ', i);
      if (e == -1) e = r.length();
      return (int)r.substring(i + k.length() + 1).toInt();
    };
    quietThresh  = getInt(req, "q", quietThresh);
    noisyThresh  = getInt(req, "n", noisyThresh);
    freqBands[0] = getInt(req, "f0", freqBands[0]);
    freqBands[1] = getInt(req, "f1", freqBands[1]);
    freqBands[2] = getInt(req, "f2", freqBands[2]);
    freqBands[3] = getInt(req, "f3", freqBands[3]);
    savePrefs();
    Serial.println("[Web] Audio tuning saved");
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
  loadPrefs();           // restore brightness, mode, custom profile from NVS
  applyActiveColors();   // fill activeColors[] from loaded settings
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(0);          // start dark — standby mode
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  currentBright = 0;                 // prevent fade-out logic from triggering
  startWiFiAP();
  startWebServer();
  setupI2S();
  Serial.printf("[Lamp] Emotion Lamp v3.0 — brightness=%d mode=%s\n",
    (int)userBrightness, activeMode==MODE_DEFAULT?"default":"custom");
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

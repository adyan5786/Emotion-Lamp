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
#include <ESPmDNS.h>
#include <FastLED.h>
#include <arduinoFFT.h>
#include <Preferences.h>   // ESP32 NVS — persists across power cycles
#include "config.h"
#include "webui.h"

#define FW_VERSION "v3.1"

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
enum AnimStyle {
  ANIM_SOLID   = 0,
  ANIM_MATRIX  = 1,
  ANIM_TWINKLE = 2,
  ANIM_RIPPLE  = 3,
  ANIM_WIPE    = 4
};
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

// ── Network settings (Phase 0/1 foundation) ─────────────────
struct NetworkSettings {
  char apSsid[33];     // 1..32 chars
  char apPass[65];     // 8..63 chars or empty for open AP
  uint8_t apIp[4];
  char staSsid[33];    // optional
  char staPass[65];    // optional if staSsid empty
  char hostname[33];   // for future mDNS support
};

NetworkSettings netCfg;
NetworkSettings appliedNetCfg;
bool staConnected = false;

// ── mDNS runtime state (Phase 7) ────────────────────────────
bool mdnsRunning = false;
bool mdnsHealthy = false;
uint8_t mdnsFailCount = 0;
unsigned long mdnsLastCheckMs = 0;
unsigned long mdnsLastRetryMs = 0;

#define STA_CONNECT_TIMEOUT_MS 12000
#define MDNS_HEALTH_INTERVAL_MS 5000
#define MDNS_RETRY_INTERVAL_MS 10000

void copyStrSafe(char* dst, size_t dstSize, const char* src) {
  if (!dst || dstSize == 0) return;
  if (!src) src = "";
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
}

void copyNetworkSettings(NetworkSettings& dst, const NetworkSettings& src) {
  copyStrSafe(dst.apSsid, sizeof(dst.apSsid), src.apSsid);
  copyStrSafe(dst.apPass, sizeof(dst.apPass), src.apPass);
  for (int i = 0; i < 4; i++) dst.apIp[i] = src.apIp[i];
  copyStrSafe(dst.staSsid, sizeof(dst.staSsid), src.staSsid);
  copyStrSafe(dst.staPass, sizeof(dst.staPass), src.staPass);
  copyStrSafe(dst.hostname, sizeof(dst.hostname), src.hostname);
}

bool networkSettingsEqual(const NetworkSettings& a, const NetworkSettings& b) {
  return strcmp(a.apSsid, b.apSsid) == 0 &&
         strcmp(a.apPass, b.apPass) == 0 &&
         a.apIp[0] == b.apIp[0] &&
         a.apIp[1] == b.apIp[1] &&
         a.apIp[2] == b.apIp[2] &&
         a.apIp[3] == b.apIp[3] &&
         strcmp(a.staSsid, b.staSsid) == 0 &&
         strcmp(a.staPass, b.staPass) == 0 &&
         strcmp(a.hostname, b.hostname) == 0;
}

bool isNetworkRestartRequired() {
  return !networkSettingsEqual(netCfg, appliedNetCfg);
}

void clearStaCredentials() {
  netCfg.staSsid[0] = '\0';
  netCfg.staPass[0] = '\0';
}

void loadNetworkDefaults() {
  copyStrSafe(netCfg.apSsid, sizeof(netCfg.apSsid), AP_SSID);
  copyStrSafe(netCfg.apPass, sizeof(netCfg.apPass), AP_PASSWORD);
  netCfg.apIp[0] = AP_IP_1;
  netCfg.apIp[1] = AP_IP_2;
  netCfg.apIp[2] = AP_IP_3;
  netCfg.apIp[3] = AP_IP_4;
  netCfg.staSsid[0] = '\0';
  netCfg.staPass[0] = '\0';
  copyStrSafe(netCfg.hostname, sizeof(netCfg.hostname), "emotionlamp");
}

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
  loadNetworkDefaults();
  prefs.begin("elamp", true);
  userBrightness = prefs.getUChar("brightness", BRIGHT_MAX);
  activeMode     = (LampMode)constrain(prefs.getUChar("mode", 0), 0, 5);
  activeAnim     = (AnimStyle)constrain(prefs.getUChar("anim", 0), 0, 4);
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

  String s;
  s = prefs.getString("ap_ssid", netCfg.apSsid);
  if (s.length() > 0 && s.length() <= 32) copyStrSafe(netCfg.apSsid, sizeof(netCfg.apSsid), s.c_str());

  s = prefs.getString("ap_pass", netCfg.apPass);
  if ((s.length() >= 8 && s.length() <= 63) || s.length() == 0)
    copyStrSafe(netCfg.apPass, sizeof(netCfg.apPass), s.c_str());

  netCfg.apIp[0] = (uint8_t)constrain((int)prefs.getUChar("ap_ip1", netCfg.apIp[0]), 0, 255);
  netCfg.apIp[1] = (uint8_t)constrain((int)prefs.getUChar("ap_ip2", netCfg.apIp[1]), 0, 255);
  netCfg.apIp[2] = (uint8_t)constrain((int)prefs.getUChar("ap_ip3", netCfg.apIp[2]), 0, 255);
  netCfg.apIp[3] = (uint8_t)constrain((int)prefs.getUChar("ap_ip4", netCfg.apIp[3]), 1, 254);

  s = prefs.getString("sta_ssid", "");
  if (s.length() <= 32) copyStrSafe(netCfg.staSsid, sizeof(netCfg.staSsid), s.c_str());

  s = prefs.getString("sta_pass", "");
  if (s.length() <= 63) copyStrSafe(netCfg.staPass, sizeof(netCfg.staPass), s.c_str());

  s = prefs.getString("host", netCfg.hostname);
  if (s.length() > 0 && s.length() <= 32) copyStrSafe(netCfg.hostname, sizeof(netCfg.hostname), s.c_str());

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

  prefs.putString("ap_ssid", netCfg.apSsid);
  prefs.putString("ap_pass", netCfg.apPass);
  prefs.putUChar("ap_ip1", netCfg.apIp[0]);
  prefs.putUChar("ap_ip2", netCfg.apIp[1]);
  prefs.putUChar("ap_ip3", netCfg.apIp[2]);
  prefs.putUChar("ap_ip4", netCfg.apIp[3]);
  prefs.putString("sta_ssid", netCfg.staSsid);
  prefs.putString("sta_pass", netCfg.staPass);
  prefs.putString("host", netCfg.hostname);

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
  static AnimStyle lastAnim = ANIM_SOLID;
  static int wipePos = 0;
  static int wipeDir = 1;
  static unsigned long wipeLastStepMs = 0;
  static bool rippleActive = false;
  static int rippleCenter = 0;
  static int rippleStep = 0;
  static CRGB rippleColor = CRGB::Black;

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

  bool animChanged = (activeAnim != lastAnim);
  if (animChanged) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    lastMatrixUpdate = 0;
    wipePos = 0;
    wipeDir = 1;
    wipeLastStepMs = 0;
    rippleActive = false;
    rippleStep = 0;
    lastAnim = activeAnim;
  }

  switch (activeAnim) {
    case ANIM_SOLID:
      fill_solid(leds, NUM_LEDS, currentColor);
      break;

    case ANIM_MATRIX:
      if (millis() - lastMatrixUpdate > MATRIX_SPEED_MS) {
        lastMatrixUpdate = millis();
        for (int i = NUM_LEDS - 1; i > 0; i--) {
          leds[i] = leds[i - 1];
        }
        leds[0] = currentColor;
      }
      break;

    case ANIM_TWINKLE: {
      fadeToBlackBy(leds, NUM_LEDS, 40);
      uint8_t spawnChance = (rms < quietThresh) ? 30 : 120;
      uint8_t twinkles = (uint8_t)constrain(map((long)rms, quietThresh, noisyThresh, 1, 4), 1, 4);
      for (uint8_t i = 0; i < twinkles; i++) {
        if (random8() < spawnChance) {
          int idx = random16(NUM_LEDS);
          CRGB c = currentColor;
          c.nscale8_video((rms < quietThresh) ? 110 : 255);
          leds[idx] = c;
        }
      }
      break;
    }

    case ANIM_RIPPLE: {
      fadeToBlackBy(leds, NUM_LEDS, 55);
      bool trigger = (!rippleActive) || ((rms > ((quietThresh + noisyThresh) / 2)) && random8() < 35);
      if (trigger) {
        rippleCenter = random16(NUM_LEDS);
        rippleStep = 0;
        rippleColor = currentColor;
        rippleActive = true;
      }
      if (rippleActive) {
        int maxStep = max(1, NUM_LEDS / 2);
        uint8_t fadeLevel = qsub8(255, (uint8_t)(rippleStep * (255 / maxStep)));
        CRGB c = rippleColor;
        c.nscale8_video(fadeLevel);
        int i1 = (rippleCenter + rippleStep) % NUM_LEDS;
        int i2 = (rippleCenter - rippleStep + NUM_LEDS) % NUM_LEDS;
        leds[i1] = c;
        leds[i2] = c;
        rippleStep++;
        if (rippleStep > maxStep) rippleActive = false;
      }
      break;
    }

    case ANIM_WIPE: {
      fadeToBlackBy(leds, NUM_LEDS, 35);
      uint16_t stepMs = (uint16_t)constrain(map((long)rms, quietThresh, noisyThresh, 120, 30), 25, 140);
      if (millis() - wipeLastStepMs >= stepMs) {
        wipeLastStepMs = millis();
        wipePos = (wipePos + wipeDir + NUM_LEDS) % NUM_LEDS;
      }
      if (rms >= noisyThresh && random8() < 25) wipeDir = -wipeDir;
      leds[wipePos] = currentColor;
      int tail = (wipePos - wipeDir + NUM_LEDS) % NUM_LEDS;
      CRGB t = currentColor;
      t.nscale8_video(145);
      leds[tail] = t;
      break;
    }

    default:
      fill_solid(leds, NUM_LEDS, currentColor);
      break;
  }

  FastLED.show();
}

// ── Section 3: WiFi AP + Web Server ──────────────────────────
void startWiFiAP() {
  IPAddress apIP   (netCfg.apIp[0], netCfg.apIp[1], netCfg.apIp[2], netCfg.apIp[3]);
  IPAddress gateway(netCfg.apIp[0], netCfg.apIp[1], netCfg.apIp[2], netCfg.apIp[3]);
  IPAddress subnet (255, 255, 255, 0);
  Serial.println("[WiFi] Starting Access Point...");
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(netCfg.apSsid, netCfg.apPass);
  Serial.printf("[WiFi] AP SSID: %s\n", netCfg.apSsid);
  Serial.print("[WiFi] AP IP: ");
  Serial.println(WiFi.softAPIP());
}

bool hasStaCredentials() {
  return strlen(netCfg.staSsid) > 0;
}

void stopMDNS() {
  if (mdnsRunning) {
    MDNS.end();
    mdnsRunning = false;
  }
  mdnsHealthy = false;
}

bool startMDNS() {
  if (WiFi.status() != WL_CONNECTED || strlen(netCfg.hostname) == 0) {
    stopMDNS();
    return false;
  }

  if (mdnsRunning) MDNS.end();

  if (!MDNS.begin(netCfg.hostname)) {
    mdnsRunning = false;
    mdnsHealthy = false;
    mdnsFailCount++;
    Serial.println("[mDNS] Start failed. Will retry without blocking boot.");
    return false;
  }

  MDNS.setInstanceName("Emotion Lamp");
  MDNS.addService("http", "tcp", 80);
  mdnsRunning = true;
  mdnsHealthy = true;
  mdnsLastCheckMs = millis();
  mdnsFailCount = 0;
  Serial.printf("[mDNS] Ready at http://%s.local\n", netCfg.hostname);
  return true;
}

void handleMDNS() {
  unsigned long now = millis();
  bool staUp = (WiFi.status() == WL_CONNECTED);

  if (!staUp) {
    if (mdnsRunning) {
      Serial.println("[mDNS] STA down. Stopping mDNS.");
      stopMDNS();
    }
    return;
  }

  if (!mdnsRunning) {
    if ((now - mdnsLastRetryMs) >= MDNS_RETRY_INTERVAL_MS) {
      mdnsLastRetryMs = now;
      startMDNS();
    }
    return;
  }

  if ((now - mdnsLastCheckMs) >= MDNS_HEALTH_INTERVAL_MS) {
    mdnsLastCheckMs = now;
    // ESP32 MDNSResponder has no update() API; keep a lightweight heartbeat tick.
    mdnsHealthy = true;
  }
}

bool connectSTAWithTimeout(uint32_t timeoutMs) {
  if (!hasStaCredentials()) {
    Serial.println("[WiFi] STA skipped: no saved STA SSID.");
    return false;
  }

  // Apply DHCP hostname for STA before attempting association.
  if (strlen(netCfg.hostname) > 0) {
    WiFi.setHostname(netCfg.hostname);
  }

  Serial.printf("[WiFi] STA connect attempt -> SSID: %s\n", netCfg.staSsid);
  Serial.printf("[WiFi] STA hostname      -> %s\n", netCfg.hostname);

  WiFi.disconnect(false, false);
  if (strlen(netCfg.staPass) > 0) WiFi.begin(netCfg.staSsid, netCfg.staPass);
  else                            WiFi.begin(netCfg.staSsid);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] STA connected.");
    Serial.print("[WiFi] STA IP: ");
    Serial.println(WiFi.localIP());
    startMDNS();
    return true;
  }

  Serial.println("[WiFi] STA failed/timed out. AP fallback remains active.");
  WiFi.disconnect(false, false);
  stopMDNS();
  return false;
}

void logNetworkState() {
  wifi_mode_t m = WiFi.getMode();
  const char* modeStr = "UNKNOWN";
  if (m == WIFI_MODE_NULL)     modeStr = "NULL";
  else if (m == WIFI_MODE_STA) modeStr = "STA";
  else if (m == WIFI_MODE_AP)  modeStr = "AP";
  else if (m == WIFI_MODE_APSTA) modeStr = "AP+STA";

  Serial.printf("[WiFi] Mode: %s\n", modeStr);
  Serial.printf("[WiFi] AP  : SSID=%s IP=%s clients=%d\n",
    netCfg.apSsid,
    WiFi.softAPIP().toString().c_str(),
    WiFi.softAPgetStationNum());
  Serial.printf("[WiFi] STA : status=%s SSID=%s IP=%s RSSI=%d\n",
    (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected",
    netCfg.staSsid,
    (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "-",
    (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0);
}

void startWebServer() {
  server.begin();
  Serial.printf("[Web]  Server at http://%d.%d.%d.%d\n",
    (int)netCfg.apIp[0], (int)netCfg.apIp[1], (int)netCfg.apIp[2], (int)netCfg.apIp[3]);
}

int hexToNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  return -1;
}

String urlDecode(const String& encoded) {
  String decoded;
  decoded.reserve(encoded.length());
  for (size_t i = 0; i < encoded.length(); i++) {
    char c = encoded[i];
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < encoded.length()) {
      int hi = hexToNibble(encoded[i + 1]);
      int lo = hexToNibble(encoded[i + 2]);
      if (hi >= 0 && lo >= 0) {
        decoded += (char)((hi << 4) | lo);
        i += 2;
      } else {
        decoded += c;
      }
    } else {
      decoded += c;
    }
  }
  return decoded;
}

bool getQueryParam(const String& req, const char* key, String& value) {
  String token = String(key) + "=";
  int start = req.indexOf(token);
  if (start == -1) return false;
  start += token.length();

  int amp = req.indexOf('&', start);
  int sp  = req.indexOf(' ', start);
  int end = -1;
  if (amp == -1 && sp == -1) end = req.length();
  else if (amp == -1) end = sp;
  else if (sp == -1) end = amp;
  else end = min(amp, sp);

  value = req.substring(start, end);
  return true;
}

bool getDecodedParam(const String& req, const char* key, String& value) {
  String raw;
  if (!getQueryParam(req, key, raw)) return false;
  value = urlDecode(raw);
  return true;
}

bool isValidSsid(const String& ssid, bool allowEmpty) {
  if (ssid.length() == 0) return allowEmpty;
  return ssid.length() <= 32;
}

bool isValidPassword(const String& pass, bool allowEmpty) {
  if (pass.length() == 0) return allowEmpty;
  return pass.length() >= 8 && pass.length() <= 63;
}

bool isValidHostname(const String& host) {
  if (host.length() == 0 || host.length() > 32) return false;
  for (size_t i = 0; i < host.length(); i++) {
    char c = host[i];
    bool ok = (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-';
    if (!ok) return false;
  }
  return true;
}

bool parseIPv4(const String& ip, uint8_t out[4]) {
  int parts[4] = {0,0,0,0};
  int part = 0;
  String tok = "";
  for (size_t i = 0; i <= ip.length(); i++) {
    char c = (i < ip.length()) ? ip[i] : '.';
    if (c == '.') {
      if (tok.length() == 0 || part > 3) return false;
      int v = tok.toInt();
      if (v < 0 || v > 255) return false;
      parts[part++] = v;
      tok = "";
    } else if (c >= '0' && c <= '9') {
      tok += c;
    } else {
      return false;
    }
  }
  if (part != 4) return false;
  if (parts[0] == 0 || parts[0] == 255) return false;
  if (parts[3] == 0 || parts[3] == 255) return false;
  for (int i = 0; i < 4; i++) out[i] = (uint8_t)parts[i];
  return true;
}

void sendTextResponse(WiFiClient& client, int statusCode, const char* statusText, const char* body) {
  client.printf(
    "HTTP/1.1 %d %s\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n\r\n%s",
    statusCode, statusText, body ? body : "");
}

void sendJsonEscaped(WiFiClient& client, const char* s);

void sendSysInfo(WiFiClient& client) {
  bool staUp = (WiFi.status() == WL_CONNECTED);
  uint32_t upSec = millis() / 1000UL;
  uint32_t hh = upSec / 3600UL;
  uint32_t mm = (upSec % 3600UL) / 60UL;
  uint32_t ss = upSec % 60UL;
  uint32_t sketchSize = (uint32_t)ESP.getSketchSize();
  uint32_t flashSize  = (uint32_t)ESP.getFlashChipSize();
  uint32_t flashFree  = (flashSize > sketchSize) ? (flashSize - sketchSize) : 0;
  float usedPct = (flashSize > 0) ? (100.0f * (float)sketchSize / (float)flashSize) : 0.0f;
  String chipModel = ESP.getChipModel();
  const char* sdkVersion = ESP.getSdkVersion();

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Connection: close\r\n\r\n");

  client.printf(
    "{\"firmware\":{\"version\":\"%s\",\"buildDate\":\"%s\",\"buildTime\":\"%s\",\"sdk\":\"%s\"},"
    "\"chip\":{\"model\":\"%s\",\"revision\":%d,\"cpuMHz\":%d},"
    "\"memory\":{\"freeHeap\":%u,\"minFreeHeap\":%u,\"sketchSize\":%u,\"flashSize\":%u,\"flashSpeed\":%u},"
    "\"storage\":{\"firmwareUsed\":%u,\"firmwareTotal\":%u,\"firmwareFree\":%u,\"firmwareUsedPct\":%.2f},"
    "\"uptime\":{\"seconds\":%lu,\"pretty\":\"%02lu:%02lu:%02lu\"},",
    FW_VERSION, __DATE__, __TIME__, ESP.getSdkVersion(),
    chipModel.c_str(),
    (int)ESP.getChipRevision(),
    (int)ESP.getCpuFreqMHz(),
    (unsigned int)ESP.getFreeHeap(),
    (unsigned int)ESP.getMinFreeHeap(),
    (unsigned int)sketchSize,
    (unsigned int)flashSize,
    (unsigned int)ESP.getFlashChipSpeed(),
    (unsigned int)sketchSize,
    (unsigned int)flashSize,
    (unsigned int)flashFree,
    (double)usedPct,
    (unsigned long)upSec,
    (unsigned long)hh, (unsigned long)mm, (unsigned long)ss
  );

  client.print("\"network\":{\"apSsid\":\"");
  sendJsonEscaped(client, netCfg.apSsid);
  client.printf("\",\"apIp\":\"%s\",\"fallbackApIp\":\"%d.%d.%d.%d\",\"apClients\":%d,",
    WiFi.softAPIP().toString().c_str(),
    (int)netCfg.apIp[0], (int)netCfg.apIp[1], (int)netCfg.apIp[2], (int)netCfg.apIp[3],
    (int)WiFi.softAPgetStationNum());
  client.print("\"staSsid\":\"");
  sendJsonEscaped(client, netCfg.staSsid);
  client.printf("\",\"staConnected\":%s,\"staIp\":\"%s\",\"hostname\":\"",
    staUp ? "true" : "false",
    staUp ? WiFi.localIP().toString().c_str() : "");
  sendJsonEscaped(client, netCfg.hostname);
  String mdnsHost = (strlen(netCfg.hostname) > 0) ? (String(netCfg.hostname) + ".local") : "";
  client.print("\",\"mdns\":{");
  client.printf("\"running\":%s,\"healthy\":%s,\"host\":\"",
    mdnsRunning ? "true" : "false",
    mdnsHealthy ? "true" : "false");
  sendJsonEscaped(client, mdnsHost.c_str());
  client.printf("\",\"failCount\":%u}}}", (unsigned int)mdnsFailCount);
}

void sendNetStatus(WiFiClient& client) {
  bool staUp = (WiFi.status() == WL_CONNECTED);
  bool apUp  = (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA);
  bool pendingRestart = isNetworkRestartRequired();

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Connection: close\r\n\r\n");

  client.print("{\"ap\":{");
  client.printf("\"enabled\":%s,\"ssid\":\"", apUp ? "true" : "false");
  sendJsonEscaped(client, netCfg.apSsid);
  client.printf("\",\"ip\":\"%s\",\"clients\":%d},",
    WiFi.softAPIP().toString().c_str(),
    (int)WiFi.softAPgetStationNum());

  client.print("\"sta\":{");
  client.printf("\"configured\":%s,\"connected\":%s,\"ssid\":\"",
    hasStaCredentials() ? "true" : "false",
    staUp ? "true" : "false");
  sendJsonEscaped(client, netCfg.staSsid);
  client.printf("\",\"ip\":\"%s\",\"rssi\":%d},",
    staUp ? WiFi.localIP().toString().c_str() : "",
    staUp ? WiFi.RSSI() : 0);

  client.print("\"hostname\":\"");
  sendJsonEscaped(client, netCfg.hostname);
  String mdnsHost = (strlen(netCfg.hostname) > 0) ? (String(netCfg.hostname) + ".local") : "";
  client.printf("\",\"mdns\":{\"running\":%s,\"healthy\":%s,\"host\":\"",
    mdnsRunning ? "true" : "false",
    mdnsHealthy ? "true" : "false");
  sendJsonEscaped(client, mdnsHost.c_str());
  client.printf("\",\"failCount\":%u},\"fallbackApIp\":\"%d.%d.%d.%d\",\"restartRequired\":%s,\"pendingRestart\":%s}",
    (unsigned int)mdnsFailCount,
    (int)netCfg.apIp[0], (int)netCfg.apIp[1], (int)netCfg.apIp[2], (int)netCfg.apIp[3],
    pendingRestart ? "true" : "false",
    pendingRestart ? "true" : "false");
}

// Writes a JSON-safe string directly to the socket without building
// a large temporary buffer in RAM.
void sendJsonEscaped(WiFiClient& client, const char* s) {
  if (!s) return;
  while (*s) {
    char c = *s++;
    switch (c) {
      case '"': client.print("\\\""); break;
      case '\\': client.print("\\\\"); break;
      case '\n': client.print("\\n"); break;
      case '\r': client.print("\\r"); break;
      case '\t': client.print("\\t"); break;
      default:
        if ((uint8_t)c < 0x20) {
          char hex[3];
          snprintf(hex, sizeof(hex), "%02x", (uint8_t)c);
          client.print("\\u00");
          client.print(hex);
        } else {
          client.write((uint8_t)c);
        }
        break;
    }
  }
}

// Sends JSON with current lamp state — fetched by the webpage on load
// and periodically for live RMS/Freq display.
void sendStatus(WiFiClient& client) {
  static const char* ZONE_NAMES[5] = {
    "Bass", "Low Speech", "Mid Speech", "Upper Speech", "Loud/Shrill"
  };
  static const char* ZONE_DESCS[5] = {
    "Deep bass and low rumbles",
    "Low voices and soft hums",
    "Normal conversation",
    "Raised voices and high tones",
    "Loud or shrill sounds"
  };

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Connection: close\r\n\r\n");

  client.printf(
    "{\"on\":%s,\"brightness\":%d,\"anim\":%d,\"mode\":%d,"
    "\"audio\":{\"q\":%d,\"n\":%d,\"f\":[%d,%d,%d,%d]},",
    audioReactiveEnabled ? "true" : "false",
    (int)userBrightness,
    (int)activeAnim,
    (int)activeMode,
    quietThresh,
    noisyThresh,
    freqBands[0], freqBands[1], freqBands[2], freqBands[3]
  );

  client.print("\"profileNames\":[");
  for (int p = 0; p < 5; p++) {
    if (p > 0) client.print(',');
    client.print('"');
    sendJsonEscaped(client, customProfiles[p].name);
    client.print('"');
  }
  client.print("],");

  client.print("\"allProfiles\":[");
  for (int p = 0; p < 5; p++) {
    if (p > 0) client.print(',');
    client.print('[');
    for (int z = 0; z < 5; z++) {
      if (z > 0) client.print(',');
      client.printf("[%d,%d,%d]",
        (int)customProfiles[p].zones[z].r,
        (int)customProfiles[p].zones[z].g,
        (int)customProfiles[p].zones[z].b);
    }
    client.print(']');
  }
  client.print("],");

  client.print("\"zones\":[");
  client.print("{\"name\":\"Quiet Room\",\"desc\":\"Silence or near-silence\",\"hex\":\"#ffffff\",\"label\":\"(255,255,255)\"}");
  for (int i = 0; i < 5; i++) {
    client.printf(
      ",{\"name\":\"%s\",\"desc\":\"%s\",\"hex\":\"#%02x%02x%02x\",\"label\":\"(%d,%d,%d)\"}",
      ZONE_NAMES[i], ZONE_DESCS[i],
      (int)activeColors[i].r, (int)activeColors[i].g, (int)activeColors[i].b,
      (int)activeColors[i].r, (int)activeColors[i].g, (int)activeColors[i].b
    );
  }
  client.print("]}");
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

  // ── Route: /netstatus — runtime AP/STA state for UI
  } else if (req.indexOf("/netstatus") != -1) {
    sendNetStatus(client);

  // ── Route: /sysinfo — live system specifications
  } else if (req.indexOf("/sysinfo") != -1) {
    sendSysInfo(client);

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

  // ── Route: /setanim?a=N (0=solid, 1=matrix, 2=twinkle, 3=ripple, 4=wipe)
  } else if (req.indexOf("/setanim") != -1) {
    int ai=req.indexOf("a=");
    if (ai!=-1) {
      activeAnim=(AnimStyle)constrain(req.substring(ai+2).toInt(),0,4);
      savePrefs();
      Serial.printf("[Web] Anim->%d\n",(int)activeAnim);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /setname?p=N&name=... — update a profile's display name
  } else if (req.indexOf("/setname") != -1) {
    String pRaw, nameRaw;
    if (getQueryParam(req, "p", pRaw) && getDecodedParam(req, "name", nameRaw)) {
      int p = constrain(pRaw.toInt(), 0, 4);
      String nm = nameRaw;
      nm.trim();
      if (nm.length() > 11) nm = nm.substring(0, 11);
      strncpy(customProfiles[p].name, nm.c_str(), sizeof(customProfiles[p].name) - 1);
      customProfiles[p].name[sizeof(customProfiles[p].name) - 1] = '\0';
      Serial.printf("[Web] Profile %d name->%s\n", p, customProfiles[p].name);
    }
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK");

  // ── Route: /netcfg — returns persisted network settings (passwords masked)
  } else if (req.indexOf("/netcfg") != -1) {
    int apPassLen  = strlen(netCfg.apPass);
    int staPassLen = strlen(netCfg.staPass);
    bool staUp = (WiFi.status() == WL_CONNECTED);
    bool pendingRestart = isNetworkRestartRequired();
    client.print(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Connection: close\r\n\r\n");
    client.print("{\"ap\":{\"ssid\":\"");
    sendJsonEscaped(client, netCfg.apSsid);
    client.printf("\",\"passLen\":%d,\"ip\":\"%d.%d.%d.%d\"},",
      apPassLen,
      (int)netCfg.apIp[0], (int)netCfg.apIp[1], (int)netCfg.apIp[2], (int)netCfg.apIp[3]);
    client.print("\"sta\":{\"ssid\":\"");
    sendJsonEscaped(client, netCfg.staSsid);
    client.printf("\",\"passLen\":%d,\"connected\":%s,\"ip\":\"%s\"},\"host\":\"",
      staPassLen,
      staUp ? "true" : "false",
      staUp ? WiFi.localIP().toString().c_str() : "");
    sendJsonEscaped(client, netCfg.hostname);
    client.printf("\",\"pendingRestart\":%s}", pendingRestart ? "true" : "false");

  // ── Route: /setwifiap?ssid=...&pass=...&ip=a.b.c.d
  } else if (req.indexOf("/setwifiap") != -1) {
    String ssid, pass, ip;
    if (!getDecodedParam(req, "ssid", ssid) || !getDecodedParam(req, "pass", pass) || !getDecodedParam(req, "ip", ip)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: missing ssid/pass/ip");
    } else if (!isValidSsid(ssid, false)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: invalid AP SSID (1-32 chars)");
    } else if (!isValidPassword(pass, true)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: invalid AP password (empty or 8-63 chars)");
    } else {
      uint8_t parsed[4];
      if (!parseIPv4(ip, parsed)) {
        sendTextResponse(client, 400, "Bad Request", "ERR: invalid AP IP");
      } else {
        copyStrSafe(netCfg.apSsid, sizeof(netCfg.apSsid), ssid.c_str());
        copyStrSafe(netCfg.apPass, sizeof(netCfg.apPass), pass.c_str());
        for (int i = 0; i < 4; i++) netCfg.apIp[i] = parsed[i];
        savePrefs();
        sendTextResponse(client, 200, "OK", "OK: AP settings saved (reboot to apply)");
      }
    }

  // ── Route: /setwifista?ssid=...&pass=...
  } else if (req.indexOf("/setwifista") != -1) {
    String ssid, pass;
    if (!getDecodedParam(req, "ssid", ssid) || !getDecodedParam(req, "pass", pass)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: missing ssid/pass");
    } else if (!isValidSsid(ssid, true)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: invalid STA SSID (0-32 chars)");
    } else if (ssid.length() > 0 && !isValidPassword(pass, true)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: invalid STA password (empty or 8-63 chars)");
    } else {
      copyStrSafe(netCfg.staSsid, sizeof(netCfg.staSsid), ssid.c_str());
      copyStrSafe(netCfg.staPass, sizeof(netCfg.staPass), pass.c_str());
      savePrefs();
      sendTextResponse(client, 200, "OK", "OK: STA settings saved");
    }

  // ── Route: /forgetsta — clear stored STA credentials
  } else if (req.indexOf("/forgetsta") != -1) {
    clearStaCredentials();
    savePrefs();
    sendTextResponse(client, 200, "OK", "OK: STA credentials cleared (reboot to apply)");

  // ── Route: /sethost?name=emotionlamp
  } else if (req.indexOf("/sethost") != -1) {
    String host;
    if (!getDecodedParam(req, "name", host)) {
      sendTextResponse(client, 400, "Bad Request", "ERR: missing hostname");
    } else {
      host.trim();
      host.toLowerCase();
      if (!isValidHostname(host)) {
        sendTextResponse(client, 400, "Bad Request", "ERR: invalid hostname (a-z 0-9 -)");
      } else {
        copyStrSafe(netCfg.hostname, sizeof(netCfg.hostname), host.c_str());
        savePrefs();
        sendTextResponse(client, 200, "OK", "OK: hostname saved");
      }
    }

  // ── Route: /applynetwork (or /applyrestart) — apply pending network config
  } else if (req.indexOf("/discardnetwork") != -1) {
    if (!isNetworkRestartRequired()) {
      sendTextResponse(client, 200, "OK", "OK: no staged network changes");
    } else {
      copyNetworkSettings(netCfg, appliedNetCfg);
      savePrefs();
      sendTextResponse(client, 200, "OK", "OK: staged network changes discarded");
    }

  // ── Route: /applynetwork (or /applyrestart) — apply pending network config
  } else if (req.indexOf("/applynetwork") != -1 || req.indexOf("/applyrestart") != -1) {
    bool pendingRestart = isNetworkRestartRequired();
    if (!pendingRestart) {
      sendTextResponse(client, 200, "OK", "OK: no pending network changes");
    } else {
      sendTextResponse(client, 200, "OK", "OK: applying network config, rebooting");
      client.flush(); delay(100); client.stop(); delay(400);
      ESP.restart();
    }

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
    size_t len = strlen_P(INDEX_HTML);
    client.printf(
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
      "Content-Length: %u\r\nConnection: close\r\n\r\n", (unsigned int)len);
    char buf[256];
    size_t pos = 0;
    while (pos < len) {
      size_t chunk = min((size_t)256, (size_t)(len - pos));
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

  // Phase 2: bring AP up first (always reachable), then attempt STA.
  WiFi.mode(WIFI_AP_STA);
  startWiFiAP();
  staConnected = connectSTAWithTimeout(STA_CONNECT_TIMEOUT_MS);
  if (!staConnected) stopMDNS();
  copyNetworkSettings(appliedNetCfg, netCfg);
  logNetworkState();

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
    handleMDNS();
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
  handleMDNS();
  delay(30);
}

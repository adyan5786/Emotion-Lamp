
/*
 * ============================================================
 *  EMOTION LAMP — ESP32-WROOM-32E + INMP441 + WS2812B (16-LED Ring)
 * ============================================================
 *
 *  Behavior:
 *    - Quiet room           → Dim bright white
 *    - Soft / low-pitched   → Green
 *    - Noisy / high-pitched → Red
 *
 *  Pin wiring — verified against ESP32-WROOM-32E pinout:
 *
 *    INMP441  SCK  → GPIO22   (I2S bit-clock  / BCLK)
 *    INMP441  WS   → GPIO4    (I2S word-select / LRCLK)
 *    INMP441  SD   → GPIO26   (I2S serial data from mic)
 *    INMP441  L/R  → GND      (forces left channel output)
 *    INMP441  VDD  → 3.3 V
 *    INMP441  GND  → GND
 *
 *    WS2812B DATA  → GPIO25   (via SN74HC125N 3.3 V→5 V buffer)
 *
 *  ⚠  ESP32-WROOM-32E notes:
 *    • GPIO6–11   : used by internal flash — NEVER use these
 *    • GPIO34/35/36/39 : INPUT ONLY — do NOT use for output
 *    • All four pins above (4, 22, 25, 26) are safe bidirectional I/O
 *    • GPIO pins are NOT 5 V tolerant — the SN74HC125N buffer
 *      is required to drive the 5 V WS2812B data line safely
 *
 *  Board setting in Arduino IDE:
 *    Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
 *    (ESP32-WROOM-32E is fully compatible with this setting)
 *
 *  Required libraries (install via Arduino Library Manager):
 *    - FastLED    by Daniel Garcia
 *    - arduinoFFT by Enrique Condes  (install version 2.x)
 * ============================================================
 */

// ESP32 Arduino core v3.x / ESP-IDF v5 — use new I2S std driver
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include <FastLED.h>
#include <arduinoFFT.h>

// ── Pin Definitions — matched to your physical WLED wiring ───
// These are your ACTUAL wired connections (do not change unless
// you physically move jumper wires on your breadboard).
#define I2S_SCK_PIN  25   // GPIO25 → INMP441 SCK  (BCLK)
#define I2S_WS_PIN   26   // GPIO26 → INMP441 WS   (LRCLK / Word Select)
#define I2S_SD_PIN   22   // GPIO22 → INMP441 SD   (mic serial data)
// ⚠ TODO: Set this to whichever GPIO your green LED wire is on.
//   Check your breadboard — common values from WLED setups: 2 or 16.
#define LED_DATA_PIN  4   // GPIO2  → WS2812B DATA (via SN74HC125N buffer)
#define NUM_LEDS     16   // 16-segment WS2812B ring

// ── Audio Settings ───────────────────────────────────────────
#define SAMPLE_RATE 16000
#define SAMPLES 512  // Must be a power of 2
#define I2S_PORT I2S_NUM_0

// ── New-API channel handle ────────────────────────────────────
static i2s_chan_handle_t rx_chan = NULL;

// ── Sensitivity Thresholds (tune to your room) ───────────────
// Open Serial Monitor at 115200 baud to see live RMS & Freq values.
// ↓ Raise if white flickers on in near-silence
#define QUIET_THRESHOLD  300   // RMS below this → white (quiet room)
// ↓ Lower to make red trigger on quieter/louder sounds
//   Louder room: try 2500 | Current: 1800 | Very sensitive: 800
#define NOISY_THRESHOLD 1800  // RMS above this → full red (loud/override)

// ── Frequency colour band edges (Hz) ─────────────────────────
// These 5 values divide the spectrum into colour zones.
// Lower a value to make that colour appear sooner (at lower pitch).
// Colour map:  white → blue → cyan → purple → orange → red
//
//  Zone 1: freq < FREQ_BAND_1  → blue     (deep bass / rumble)
//  Zone 2: FREQ_BAND_1 – 2     → cyan     (low speech / hum)
//  Zone 3: FREQ_BAND_2 – 3     → purple   (mid speech)
//  Zone 4: FREQ_BAND_3 – 4     → orange   (upper speech)
//  Zone 5: freq > FREQ_BAND_4  → red      (high-pitched / shrill)
#define FREQ_BAND_1  350   // Hz  blue  → cyan
#define FREQ_BAND_2  600   // Hz  cyan  → purple
#define FREQ_BAND_3  900   // Hz  purple→ orange
#define FREQ_BAND_4 1300   // Hz  orange→ red

// ── Brightness ───────────────────────────────────────────────
// All values are 0–255.  Lower = dimmer.
// To make the lamp dimmer overall, reduce BOTH values.
// To make the lamp brighter overall, increase BOTH values.
#define BRIGHT_QUIET  18   // ← Idle brightness (dim white). Try 10–40.
#define BRIGHT_ACTIVE 80   // ← Mid brightness when active.  Try 60–120.
#define BRIGHT_MAX   140   // ← Peak brightness when loudest. Try 100–200.

// ── Smoothing ────────────────────────────────────────────────
// 0.0 = frozen, 1.0 = instant. Higher = snappier but flickery.
#define SMOOTH_FACTOR 0.15f  // Weight of new sample (0.0–1.0)

// ── Globals ──────────────────────────────────────────────────
CRGB leds[NUM_LEDS];

double vReal[SAMPLES];
double vImag[SAMPLES];
int32_t rawSamples[SAMPLES];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

// Smoothed state
float smoothedRMS = 0.0f;
float smoothedFreq = 200.0f;
CRGB currentColor = CRGB::White;
uint8_t currentBright = BRIGHT_QUIET;

// ── I2S setup using ESP-IDF v5 new API (Arduino core v3.x) ──
void setupI2S() {
  // 1. Allocate an RX channel on I2S0
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
      (i2s_port_t)I2S_PORT, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

  // 2. Configure standard Philips I2S mode for INMP441
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
  // INMP441 L/R = GND → left channel; select slot 0 (left)
  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}

// ── Read mic samples, return RMS and fill vReal[] for FFT ────
float readAudio() {
  size_t bytesRead = 0;
  // New API: i2s_channel_read instead of legacy i2s_read
  i2s_channel_read(rx_chan, rawSamples, sizeof(rawSamples), &bytesRead, portMAX_DELAY);

  double sum = 0.0;
  int count = bytesRead / sizeof(int32_t);

  for (int i = 0; i < count; i++) {
    // INMP441 left-justifies 18-bit data in a 32-bit word; shift to normalise
    int32_t s = rawSamples[i] >> 14;
    vReal[i] = (double)s;
    vImag[i] = 0.0;
    sum += (double)s * s;
  }

  // Zero-pad remainder if short read
  for (int i = count; i < SAMPLES; i++) {
    vReal[i] = 0.0;
    vImag[i] = 0.0;
  }

  return (count > 0) ? sqrtf((float)(sum / count)) : 0.0f;
}

// ── FFT dominant frequency ────────────────────────────────────
double getDominantFreq() {
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  return FFT.majorPeak();
}

// ── Lerp helper ───────────────────────────────────────────────
uint8_t lerpU8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)((float)a + t * ((float)b - (float)a));
}

CRGB lerpColor(CRGB a, CRGB b, float t) {
  return CRGB(
    lerpU8(a.r, b.r, t),
    lerpU8(a.g, b.g, t),
    lerpU8(a.b, b.b, t));
}

// ── Map frequency to colour across 5 bands ───────────────────
// Colours:  blue → cyan → purple → orange → red
// Each band blends smoothly into the next via lerp.
static const CRGB COL_BLUE   = CRGB( 10,  30, 255);  // Zone 1
static const CRGB COL_CYAN   = CRGB(  0, 220, 200);  // Zone 2
static const CRGB COL_PURPLE = CRGB(160,  20, 220);  // Zone 3
static const CRGB COL_ORANGE = CRGB(255, 100,   0);  // Zone 4
static const CRGB COL_RED    = CRGB(255,  15,  15);  // Zone 5

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

// ── Decide target color & brightness from audio state ─────────
void updateLamp(float rms, float freq) {
  CRGB targetColor;
  uint8_t targetBright;

  if (rms < QUIET_THRESHOLD) {
    // ── Quiet → dim white ──────────────────────────────────
    targetColor = CRGB(255, 255, 255);
    targetBright = BRIGHT_QUIET;

  } else if (rms >= NOISY_THRESHOLD) {
    // ── Very loud → full red regardless of frequency ───────
    targetColor = COL_RED;
    targetBright = BRIGHT_MAX;

  } else {
    // ── Active: colour driven by frequency, brightness by RMS
    targetColor = freqToColor(freq);
    targetBright = (uint8_t)constrain(
      map((long)rms, QUIET_THRESHOLD, NOISY_THRESHOLD, BRIGHT_ACTIVE, BRIGHT_MAX),
      BRIGHT_ACTIVE, BRIGHT_MAX);
  }

  // Smooth the transition
  currentColor = lerpColor(currentColor, targetColor, SMOOTH_FACTOR);
  currentBright = lerpU8(currentBright, targetBright, SMOOTH_FACTOR);

  FastLED.setBrightness(currentBright);
  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHT_QUIET);
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();

  setupI2S();
  Serial.println("Emotion Lamp ready.");
}

void loop() {
  // 1. Read mic → get RMS amplitude
  float rawRMS = readAudio();

  // 2. FFT → dominant frequency
  double rawFreq = getDominantFreq();

  // 3. Exponential smoothing (reduces flicker)
  smoothedRMS = smoothedRMS + SMOOTH_FACTOR * (rawRMS - smoothedRMS);
  smoothedFreq = smoothedFreq + SMOOTH_FACTOR * ((float)rawFreq - smoothedFreq);

  // 4. Debug output — open Serial Monitor at 115200 to calibrate
  Serial.print("RMS: ");
  Serial.print((int)smoothedRMS);
  Serial.print("  Freq: ");
  Serial.print((int)smoothedFreq);
  Serial.println(" Hz");

  // 5. Update LEDs
  updateLamp(smoothedRMS, smoothedFreq);

  // Small guard delay
  delay(30);
}

#pragma once

/*
 * ============================================================
 *  config.h — All user-tunable settings in one place
 *
 *  This is the ONLY file you should normally need to edit.
 *  Change values here; everything else reads from here.
 * ============================================================
 */

// ─────────────────────────────────────────────────────────────
//  ★  WiFi Access Point Credentials
// ─────────────────────────────────────────────────────────────
#define AP_SSID     "Emotion-Lamp"  // Network name (broadcast)
#define AP_PASSWORD "akatsuki"      // Min 8 characters

// ─────────────────────────────────────────────────────────────
//  ★  Web Interface IP Address
//     Visit this address after connecting to the AP above.
//     Gateway must match apIP. Subnet must stay 255.255.255.0.
// ─────────────────────────────────────────────────────────────
#define AP_IP_1  4
#define AP_IP_2  3
#define AP_IP_3  2
#define AP_IP_4  1   // → http://4.3.2.1

// ─────────────────────────────────────────────────────────────
//  Pin Definitions
//  Do NOT change unless you physically rewire the breadboard.
// ─────────────────────────────────────────────────────────────
#define I2S_SCK_PIN   25   // GPIO25 → INMP441 SCK  (BCLK)
#define I2S_WS_PIN    26   // GPIO26 → INMP441 WS   (LRCLK)
#define I2S_SD_PIN    22   // GPIO22 → INMP441 SD   (mic data)
#define LED_DATA_PIN   4   // GPIO4  → WS2812B DATA (via SN74HC125N buffer)
#define NUM_LEDS      16   // Number of LEDs in the ring

// ─────────────────────────────────────────────────────────────
//  Audio Settings
// ─────────────────────────────────────────────────────────────
#define SAMPLE_RATE  16000
#define SAMPLES        512   // FFT window size — must be a power of 2
#define I2S_PORT    I2S_NUM_0

// ─────────────────────────────────────────────────────────────
//  Sensitivity Thresholds
//  Open Serial Monitor at 115200 baud to see live RMS & Freq.
// ─────────────────────────────────────────────────────────────
// ↓ Raise if white flickers on in near-silence
#define QUIET_THRESHOLD  150   // RMS below this → white (quiet room)
// ↓ Lower to trigger red sooner  |  raise for louder rooms
//   Very sensitive: 800  |  Current: 1800  |  Loud room: 2500
#define NOISY_THRESHOLD 1800   // RMS above this → full red (volume override)

// ─────────────────────────────────────────────────────────────
//  Frequency Colour Band Edges (Hz)
//  Colour map:  white → blue → cyan → purple → orange → red
//
//  Zone 1: freq < FREQ_BAND_1  → blue     (deep bass / rumble)
//  Zone 2: FREQ_BAND_1 – 2     → cyan     (low speech / hum)
//  Zone 3: FREQ_BAND_2 – 3     → purple   (mid speech)
//  Zone 4: FREQ_BAND_3 – 4     → orange   (upper speech)
//  Zone 5: freq > FREQ_BAND_4  → red      (high-pitched / shrill)
//
//  Lower a value to shift that colour zone to lower pitches.
// ─────────────────────────────────────────────────────────────
#define FREQ_BAND_1  300   // Hz  blue   → cyan
#define FREQ_BAND_2  600   // Hz  cyan   → purple
#define FREQ_BAND_3  900   // Hz  purple → orange
#define FREQ_BAND_4 1300   // Hz  orange → red

// ─────────────────────────────────────────────────────────────
//  Brightness  (0–255, lower = dimmer)
//
//  BRIGHT_QUIET  : idle white glow when room is silent. Try 10–40.
//  BRIGHT_ACTIVE : base brightness when sound is detected. Try 60–120.
//  BRIGHT_MAX    : peak brightness at loudest sound. Try 100–200.
//
//  To dim everything: lower all three proportionally.
//  To dim only the idle state: lower BRIGHT_QUIET alone.
// ─────────────────────────────────────────────────────────────
#define BRIGHT_QUIET   18
#define BRIGHT_ACTIVE  80
#define BRIGHT_MAX    140

// ─────────────────────────────────────────────────────────────
//  Transition Smoothing
//  0.0 = frozen (no change), 1.0 = instant (no smoothing).
//  Higher = snappier response but more flicker.
// ─────────────────────────────────────────────────────────────
#define SMOOTH_FACTOR  0.15f

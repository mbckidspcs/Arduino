#include <Arduino.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// -------- SD & I2S Setup --------
#define SD_CS 5   // your SD CS pin
AudioOutputI2S *out;

const char* soundFiles[8] = {
  "/Crash1.wav", "/Hat2.wav", "/HitHat1.wav", "/Kick1.wav",
  "/Kick2.wav", "/Snare1.wav", "/Snare2.wav", "/Tamb.wav"
};

// Each pad gets its own WAV object (so overlapping works)
AudioFileSourceSD* files[8];
AudioGeneratorWAV* wavs[8];

// Track last gain (for fade)
float currentGain = 0.8f;

// ---------- Fade helper ----------
void rampGain(float target, int ms) {
  const int steps = max(1, ms / 2);
  float start = currentGain;
  for (int i = 1; i <= steps; i++) {
    float g = start + (target - start) * (float)i / steps;
    out->SetGain(g);
    delay(2);
  }
  currentGain = target;
}

// ---------- Play function ----------
void playSound(int idx) {
  if (idx < 0 || idx > 7) return;

  // If something is already running on this pad, fade out + stop
  if (wavs[idx] &&

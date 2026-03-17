#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "driver/i2s.h"

// ---- Pin Definitions ----
const int piezoPins[8] = {32, 33, 34, 35, 36, 39, 4, 2};

#define I2S_DOUT 12
#define I2S_BCLK 14
#define I2S_LRC  27
#define SD_CS    5
#define SD_MOSI  23
#define SD_MISO  19
#define SD_SCK   18

const char* soundFiles[8] = {
  "/Crash1.wav", "/Kick2.wav", "/HitHat1.wav", "/Kick1.wav",
  "/Kick2.wav", "/Snare1.wav", "/Snare2.wav", "/Tamb.wav"
};

// ---- I2S Config ----
const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
};

const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
};

#define NUM_PADS 8
#define BUFFER_SIZE 512
#define HIT_STATE 60        // Trigger on HIGH (change if you use pull-down or pull-up)
#define AUDIO_GAIN 2.0f       // Tweak for volume boost (max ~3.0)

unsigned long lastHitTime[NUM_PADS] = {0};
const unsigned long debounceDelay = 20; // ms

struct PlayingFile {
  File file;
  bool active;
  size_t bytesRead;
  byte buffer[BUFFER_SIZE];
  int bufferPos;
  bool ending;          // True if this buffer is the last (for fade-out)
  int lastSamples;      // Number of samples in last buffer (for fade-out)
};

PlayingFile playFiles[NUM_PADS];
static int16_t mixedBuf[BUFFER_SIZE / 2] = {0};
static int16_t zeroBuf[BUFFER_SIZE / 2] = {0}; // Silence buffer

void stopPad(int idx) {
  if (playFiles[idx].active) {
    playFiles[idx].file.close();
    playFiles[idx].active = false;
    playFiles[idx].bytesRead = 0;
    playFiles[idx].bufferPos = BUFFER_SIZE;
    playFiles[idx].ending = false;
    playFiles[idx].lastSamples = 0;
  }
}

void startPad(int idx) {
  stopPad(idx); // Always stop previous play if ongoing!

  File f = SD.open(soundFiles[idx]);
  if (!f) {
    Serial.printf("Error opening file: %s\n", soundFiles[idx]);
    return;
  }
  // Skip WAV header
  for (int i = 0; i < 44; i++) f.read();

  playFiles[idx].file = f;
  playFiles[idx].bytesRead = 0;
  playFiles[idx].bufferPos = BUFFER_SIZE; // trigger buffer reload
  playFiles[idx].active = true;
  playFiles[idx].ending = false;
  playFiles[idx].lastSamples = 0;
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_PADS; i++) {
    pinMode(piezoPins[i], INPUT); // Use INPUT_PULLDOWN/UP if needed
  }

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("SD Card initialized.");

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);

  for (int i = 0; i < NUM_PADS; i++) {
    playFiles[i].active = false;
    playFiles[i].ending = false;
    playFiles[i].lastSamples = 0;
  }
}

void loop() {
  for (int i = 0; i < NUM_PADS; i++) {
    int state = digitalRead(piezoPins[i]);
    
    if (state > HIT_STATE && (millis() - lastHitTime[i] > debounceDelay)) {
      Serial.printf("Pad %d hit! digital state=%d\n", i, state);
      startPad(i);
      lastHitTime[i] = millis();
    }
  }
  mixAndPlayActive();
}

void mixAndPlayActive() {
  int maxSamples = 0;
  bool anyActive = false;

  // Preload buffers and find max samples
  for (int ch = 0; ch < NUM_PADS; ch++) {
    if (!playFiles[ch].active) continue;
    anyActive = true;
    if (playFiles[ch].bufferPos >= playFiles[ch].bytesRead) {
      playFiles[ch].bytesRead = playFiles[ch].file.read(playFiles[ch].buffer, BUFFER_SIZE);
      playFiles[ch].bufferPos = 0;

      if (playFiles[ch].bytesRead == 0) {
        playFiles[ch].file.close();
        playFiles[ch].active = false;
        playFiles[ch].ending = false;
        playFiles[ch].lastSamples = 0;
        continue;
      }
      // If this is the last buffer for this pad, mark as ending for fade-out
      if (playFiles[ch].file.available() == 0) {
        playFiles[ch].ending = true;
        playFiles[ch].lastSamples = playFiles[ch].bytesRead / 2;
      } else {
        playFiles[ch].ending = false;
        playFiles[ch].lastSamples = 0;
      }
    }
    int samples = playFiles[ch].bytesRead / 2;
    if (samples > maxSamples) maxSamples = samples;
  }

  if (maxSamples == 0) {
    // No pads active: keep I2S streaming by writing silence
    size_t bytes_written;
    i2s_write(I2S_NUM_0, zeroBuf, BUFFER_SIZE, &bytes_written, portMAX_DELAY);
    return;
  }

  // Clear mixedBuf
  for (int i = 0; i < maxSamples; i++) mixedBuf[i] = 0;

  // Mix all active sounds (with fade-out for last samples)
  for (int i = 0; i < maxSamples; i++) {
    int32_t mix = 0; int sources = 0;
    for (int ch = 0; ch < NUM_PADS; ch++) {
      if (!playFiles[ch].active) continue;
      if (playFiles[ch].bufferPos + 1 < playFiles[ch].bytesRead) {
        int16_t sample = (int16_t)((playFiles[ch].buffer[playFiles[ch].bufferPos+1] << 8) | playFiles[ch].buffer[playFiles[ch].bufferPos]);
        // Fade out if this is the last buffer for this pad
        if (playFiles[ch].ending && i >= (playFiles[ch].lastSamples - 1000) && i < playFiles[ch].lastSamples) {
          float gain = float(playFiles[ch].lastSamples - i) / 1000.0f;
          if (gain < 0.0f) gain = 0.0f;
          sample = (int16_t)(sample * gain);
        }
        mix += sample;
        playFiles[ch].bufferPos += 2;
        sources++;
      }
    }
    if (sources > 1) mix /= sources;

    int amplified = (int)(mix * AUDIO_GAIN);
    if (amplified > 32767) amplified = 32767;
    if (amplified < -32768) amplified = -32768;
    mixedBuf[i] = (int16_t)amplified;
  }

  size_t bytes_written;
  i2s_write(I2S_NUM_0, mixedBuf, maxSamples * 2, &bytes_written, portMAX_DELAY);
}
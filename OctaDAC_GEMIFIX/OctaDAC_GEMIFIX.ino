  #include <Arduino.h>
  #include <SD.h>
  #include <SPI.h>
  #include "driver/i2s.h"
  #include "driver/adc.h" // Include the ADC driver for low-level control

  // ---- Pin Definitions ----
  // Please ensure these pins are on ADC1 to avoid conflicts
  // ADC1 Pins: 32, 33, 34, 35, 36, 39
  const int piezoPins[8] = {32, 33, 34, 35, 36, 39, 4, 2}; // Note: Pins 4 and 2 are on ADC2.
  // It is recommended to use only ADC1 pins like: {32, 33, 34, 35, 36, 39, 0, 2}
  // However, the provided code will now use the ADC API, which should be more stable.

  // Mapping of GPIO pins to ADC1 channels
  const adc1_channel_t adc1_channels[6] = {
      ADC1_CHANNEL_4, // GPIO 32
      ADC1_CHANNEL_5, // GPIO 33
    //  ADC1_CHANNEL_6, // GPIO 34
     // ADC1_CHANNEL_7, // GPIO 35
      ADC1_CHANNEL_0, // GPIO 36
      ADC1_CHANNEL_3, // GPIO 39
      ADC1_CHANNEL_0, // GPIO 4 - This is actually on ADC2. Using it with ADC1 will cause issues.
      ADC1_CHANNEL_2, // GPIO 2 - This is actually on ADC2. Using it with ADC1 will cause issues.
  };

  #define I2S_DOUT 12
  #define I2S_BCLK 14
  #define I2S_LRC 27
  #define SD_CS  5
  #define SD_MOSI 23
  #define SD_MISO 19
  #define SD_SCK  18

  // Array of sound file paths
  const char* soundFiles[8] = {
  "/Crash1.wav", "/Tom1.wav", "/HitHat1.wav", "/Kick1.wav",
  "/Kick2.wav", "/Snare1.wav", "/Snare2.wav", "/Tamb.wav"
  };

  // ---- Buffer and Audio Constants ----
  #define BUFFER_LEN 256 // The size of our DMA buffer (in 16-bit samples)
  #define BUFFER_SIZE_BYTES (BUFFER_LEN * 2) // Total buffer size in bytes
  #define HIT_THRESHOLD 120    // Trigger on HIGH
  #define AUDIO_GAIN 1.5f     // Tweak for volume boost (max ~3.0)
  #define FADE_SAMPLES 500     // Number of samples for a smooth fade-out

  // ---- I2S Config ----
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Use a valid interrupt flag
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN, // Now uses the compile-time constant
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

  unsigned long lastHitTime[NUM_PADS] = {0};
  const unsigned long debounceDelay = 20; // ms

  // Data structure to track each playing file
  struct PlayingFile {
  File file;
  bool active;
  size_t bytesRead;
  int bufferPos;
    bool isFading;
    int fadePosition;
  };

  // Array of PlayingFile structs for each pad
  PlayingFile playFiles[NUM_PADS];
  static int16_t mixedBuf[BUFFER_LEN] = {0};
  static int16_t zeroBuf[BUFFER_LEN] = {0}; // Silence buffer

  // A temporary buffer for reading from a single file
  static byte readBuffer[BUFFER_SIZE_BYTES];

  // ---- Function to stop a pad from playing ----
  void stopPad(int idx) {
  if (playFiles[idx].active) {
    playFiles[idx].file.close();
    playFiles[idx].active = false;
  }
  }

  // ---- Function to start playing a sound file from a pad ----
  void startPad(int idx) {
  stopPad(idx); // Always stop previous play if ongoing!

  File f = SD.open(soundFiles[idx]);
  if (!f) {
    Serial.printf("Error opening file: %s\n", soundFiles[idx]);
    return;
  }
  // Skip WAV header (44 bytes for standard WAV)
  f.seek(44, SeekSet);

  playFiles[idx].file = f;
  playFiles[idx].bytesRead = 0;
  playFiles[idx].bufferPos = 0;
  playFiles[idx].active = true;
    playFiles[idx].isFading = false;
    playFiles[idx].fadePosition = 0;
  }

  // ---- Main setup function ----
  void setup() {
  Serial.begin(115200);

  // Configure ADC1 for all channels
  adc1_config_width(ADC_WIDTH_BIT_12);
  for (int i = 0; i < NUM_PADS; i++) {
    // Configure each ADC channel for an analog pin
    adc1_config_channel_atten(adc1_channels[i], ADC_ATTEN_DB_11);
  }
  
  // SD Card initialization
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("SD Card initialized.");

  // I2S Driver setup
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);

  for (int i = 0; i < NUM_PADS; i++) {
    playFiles[i].active = false;
  }
  }

  // ---- Main loop function ----
  void loop() {
  for (int i = 0; i < NUM_PADS; i++) {
    // Read from the ADC directly using the low-level API
    int state = adc1_get_raw(adc1_channels[i]);

    if (state > HIT_THRESHOLD && (millis() - lastHitTime[i] > debounceDelay)) {
    Serial.printf("Pad %d hit! analog reading=%d\n", i, state);
    startPad(i);
    lastHitTime[i] = millis();
    }
  }
  mixAndPlayActive();
  }

  // ---- New, more robust mixing function ----
  void mixAndPlayActive() {
  // Check if any pads are currently active
  bool anyActive = false;
  for (int i = 0; i < NUM_PADS; i++) {
    if (playFiles[i].active) {
    anyActive = true;
    break;
    }
  }

  if (!anyActive) {
    // If nothing is playing, write a silent buffer to keep I2S stream active
    size_t bytes_written;
    i2s_write(I2S_NUM_0, zeroBuf, BUFFER_SIZE_BYTES, &bytes_written, portMAX_DELAY);
    return;
  }

  // Clear the mixed buffer for this frame
  for (int i = 0; i < BUFFER_LEN; i++) mixedBuf[i] = 0;

  // Mix all active sounds
  for (int ch = 0; ch < NUM_PADS; ch++) {
    if (!playFiles[ch].active) {
    continue;
    }

    // Read from the file into a temporary buffer
    size_t bytesRead = playFiles[ch].file.read(readBuffer, BUFFER_SIZE_BYTES);

      // If the file is close to ending, start a fade-out
      if (playFiles[ch].file.available() <= (FADE_SAMPLES * 2) && !playFiles[ch].isFading) {
          playFiles[ch].isFading = true;
          playFiles[ch].fadePosition = 0;
      }

    // Check if the file has reached its end
    if (bytesRead == 0) {
    stopPad(ch);
    continue;
    }

    int16_t* sampleBuffer = (int16_t*)readBuffer;
    size_t samplesRead = bytesRead / 2;
    
    // Mix the new samples into the mixedBuf
    for (size_t i = 0; i < samplesRead; i++) {
        int16_t sample = sampleBuffer[i];

        // Apply fade-out if the pad is fading
        if (playFiles[ch].isFading) {
            float gain = 1.0f - ((float)playFiles[ch].fadePosition / FADE_SAMPLES);
            if (gain < 0.0f) gain = 0.0f;
            sample = (int16_t)(sample * gain);
            playFiles[ch].fadePosition++;

            // If the fade is complete, stop the pad
            if (playFiles[ch].fadePosition >= FADE_SAMPLES) {
              stopPad(ch);
              continue;
            }
        }
    mixedBuf[i] += sample;
    }
  }

  // Normalize the mixed audio and apply gain
  int maxSources = 0;
  for(int i=0; i<NUM_PADS; i++) {
    if(playFiles[i].active) maxSources++;
  }
  if (maxSources > 1) {
    for (size_t i = 0; i < BUFFER_LEN; i++) {
    int32_t mixedSample = (int32_t)mixedBuf[i] / maxSources;
    mixedSample = (int32_t)(mixedSample * AUDIO_GAIN);
    // Clamp the value to 16-bit range
    if (mixedSample > 32767) mixedSample = 32767;
    if (mixedSample < -32768) mixedSample = -32768;
    mixedBuf[i] = (int16_t)mixedSample;
    }
  } else {
    for (size_t i = 0; i < BUFFER_LEN; i++) {
    int32_t mixedSample = (int32_t)(mixedBuf[i] * AUDIO_GAIN);
    // Clamp the value to 16-bit range
    if (mixedSample > 32767) mixedSample = 32767;
    if (mixedSample < -32768) mixedSample = -32768;
    mixedBuf[i] = (int16_t)mixedSample;
    }
  }


  // Write the mixed audio to the I2S DAC
  size_t bytes_written;
  i2s_write(I2S_NUM_0, mixedBuf, BUFFER_SIZE_BYTES, &bytes_written, portMAX_DELAY);
  }


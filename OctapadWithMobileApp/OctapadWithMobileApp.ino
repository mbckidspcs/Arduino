    #include <Arduino.h>
    #include <SD.h>
    #include <SPI.h>
    #include "driver/i2s.h"
    #include "driver/adc.h" // Include the ADC driver for low-level control
    #include <math.h> // Include for pow() function
    #include "BluetoothSerial.h" // New library for Bluetooth communication
    #include <vector> // Include for using dynamic arrays

    // ---- I2S Config ----
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Use a valid interrupt flag
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = 14,
        .ws_io_num = 27,
        .data_out_num = 12,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // ---- Pin Definitions ----
    // The ESP32 has two ADCs (Analog-to-Digital Converters).
    // ADC1 pins: 32, 33, 34, 35, 36, 39
    // ADC2 pins: 0, 2, 4, 12, 13, 14, 15, 25, 26, 27
    // Note: ADC2 pins are typically used by the Wi-Fi driver, so it's best to stick to ADC1.
    // The code has been updated to use ADC1 and the specified ADC2 pins.

    const int NUM_PADS = 8;
    const int piezoPins[NUM_PADS] = {32, 33, 34, 35, 36, 39, 4, 2};

    // It's important to know which ADC each pin belongs to.
    // We'll have two separate arrays for ADC1 and ADC2 channels.
    const adc1_channel_t adc1_channels[6] = {
        ADC1_CHANNEL_4, // GPIO 32
        ADC1_CHANNEL_5, // GPIO 33
        ADC1_CHANNEL_6, // GPIO 34
        ADC1_CHANNEL_7, // GPIO 35
        ADC1_CHANNEL_0, // GPIO 36
        ADC1_CHANNEL_3, // GPIO 39
    };

    const adc2_channel_t adc2_channels[2] = {
        ADC2_CHANNEL_0, // GPIO 4
        ADC2_CHANNEL_2, // GPIO 2
    };

    #define I2S_DOUT 12
    #define I2S_BCLK 14
    #define I2S_LRC 27
    #define SD_CS 5
    #define SD_MOSI 23
    #define SD_MISO 19
    #define SD_SCK 18

    // Array to store sound file paths. Now a String array for dynamic loading.
    // These are the default values if no config file is found.
    String soundFilePaths[NUM_PADS] = {
        "/Crash1.wav", "/Tom1.wav", "/HitHat1.wav", "/Kick1.wav",
        "/Kick2.wav", "/Snare1.wav", "/Hat2.wav", "/Snare2.wav"
    };

    // ---- Config File Constants ----
    const char* CONFIG_FILE_NAME = "/config.txt";

    // ---- Buffer and Audio Constants ----
    #define BUFFER_LEN 256 // The size of our DMA buffer (in 16-bit samples)
    #define BUFFER_SIZE_BYTES (BUFFER_LEN * 2) // Total buffer size in bytes
    #define HIT_THRESHOLD 40        // Trigger on HIGH
    #define AUDIO_GAIN 2.0f          // Global gain for the output, now set to 2
    #define FADE_SAMPLES 500         // Number of samples for a smooth fade-out
    #define MIN_VELOCITY_GAIN 0.2f   // Minimum gain for a soft hit
    #define VELOCITY_EXPONENT 2.0f   // Exponent for the velocity curve (3.0f will make it even more responsive)

    // Data structure to track each playing file
    struct PlayingFile {
        File file;
        bool active;
        size_t bytesRead;
        int bufferPos;
        bool isFading;
        int fadePosition;
        float velocityGain; // New variable to store the velocity-based gain
    };

    // Array of PlayingFile structs for each pad
    PlayingFile playFiles[NUM_PADS];
    static int16_t mixedBuf[BUFFER_LEN] = {0};
    static int16_t zeroBuf[BUFFER_LEN] = {0}; // Silence buffer

    // Global variables moved to the top to fix scope issues
    unsigned long lastHitTime[NUM_PADS] = {0};
    const unsigned long debounceDelay = 100; // ms

    // A temporary buffer for reading from a single file
    static byte readBuffer[BUFFER_SIZE_BYTES];

    // Global Bluetooth Serial object
    BluetoothSerial SerialBT;

    // ---- Function to stop a pad from playing ----
    void stopPad(int idx) {
        if (playFiles[idx].active) {
            playFiles[idx].file.close();
            playFiles[idx].active = false;
        }
    }

    // ---- Function to start playing a sound file from a pad, now with velocity ----
    void startPad(int idx, int velocity) {
        // Calculate the velocity gain with a normal exponential curve
        float normalizedVelocity = (float)(velocity - HIT_THRESHOLD) / (3500.0f - HIT_THRESHOLD);
        // Clamp the normalized velocity to be between 0 and 1
        if (normalizedVelocity < 0.0f) normalizedVelocity = 0.0f;
        if (normalizedVelocity > 1.0f) normalizedVelocity = 1.0f;
        
        // Apply the exponential curve to the normalized velocity directly
        float velocityGain = MIN_VELOCITY_GAIN + ((1.0f - MIN_VELOCITY_GAIN) * pow(normalizedVelocity, VELOCITY_EXPONENT));
        if (velocityGain > 1.0f) velocityGain = 1.0f;

        // If the pad is already playing, simply restart the sound from the beginning
        // and update the velocity gain.
        if (playFiles[idx].active) {
            // Reset the file pointer to the beginning of the sound data
            playFiles[idx].file.seek(44, SeekSet);
            playFiles[idx].isFading = false;
            playFiles[idx].fadePosition = 0;
            playFiles[idx].velocityGain = velocityGain; // Update velocity gain for the new hit
            return;
        }

        // Use the dynamic soundFilePaths array
        File f = SD.open(soundFilePaths[idx].c_str());
        if (!f) {
            Serial.printf("Error opening file: %s\n", soundFilePaths[idx].c_str());
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
        playFiles[idx].velocityGain = velocityGain; // Store the velocity gain for this playback
    }

    // ---- Function to load sound file paths from a config file ----
    void loadConfig() {
        File configFile = SD.open(CONFIG_FILE_NAME, FILE_READ);
        if (!configFile) {
            // If the file doesn't exist, create it with the default values
            Serial.println("No config file found. Creating a new one with defaults.");
            File newConfigFile = SD.open(CONFIG_FILE_NAME, FILE_WRITE);
            if (newConfigFile) {
                for (int i = 0; i < NUM_PADS; i++) {
                    newConfigFile.println(soundFilePaths[i]);
                }
                newConfigFile.close();
            } else {
                Serial.println("Error creating config file!");
            }
            return;
        }
        
        Serial.println("Loading configuration from file...");
        int padIndex = 0;
        while (configFile.available() && padIndex < NUM_PADS) {
            String line = configFile.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
                soundFilePaths[padIndex] = line;
                Serial.printf("Pad %d sound updated to: %s\n", padIndex, soundFilePaths[padIndex].c_str());
                padIndex++;
            }
        }
        configFile.close();
    }

    // ---- Function to update a single sound file path in the config file ----
    void updateConfigFile(int padNumber, const char* waveFileName) {
        if (padNumber < 0 || padNumber >= NUM_PADS) {
            Serial.println("Invalid pad number. Please use a number between 0 and 7.");
            SerialBT.println("ERROR: Invalid pad number.");
            return;
        }

        // Update the in-memory array
        soundFilePaths[padNumber] = waveFileName;
        Serial.printf("In-memory path for pad %d updated to: %s\n", padNumber, waveFileName);

        // Read all current paths to a temporary array
        String tempPaths[NUM_PADS];
        for (int i = 0; i < NUM_PADS; i++) {
            tempPaths[i] = soundFilePaths[i];
        }

        // Rewrite the entire config file
        File configFile = SD.open(CONFIG_FILE_NAME, FILE_WRITE);
        if (!configFile) {
            Serial.println("Error opening config file for writing!");
            SerialBT.println("ERROR: Could not write to config file.");
            return;
        }
        
        for (int i = 0; i < NUM_PADS; i++) {
            configFile.println(tempPaths[i]);
        }
        configFile.close();
        Serial.println("Config file updated successfully.");
    //  SerialBT.println("Config file updated successfully.");
    }

    // ---- New function to get a list of WAV files from the SD card, now returning a comma-separated string ----
    String getWavFileList() {
        String fileList = "";
        File root = SD.open("/");
        if (!root) {
            return "ERROR: Failed to open SD card.";
        }
        
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String fileName = file.name();
                // Check if the file has a ".wav" extension (case-insensitive)
                if (fileName.endsWith(".wav") || fileName.endsWith(".WAV")) {
                    if (fileList.length() > 0) {
                        fileList += ",";
                    }
                    fileList += fileName;
                }
            }
            file = root.openNextFile();
        }
        root.close();
        return fileList;
    }

    // ---- New function to get assigned sounds from the config file, returning a comma-separated string ----
    String getAssignedSounds() {
        String assignedList = "";
        for (int i = 0; i < NUM_PADS; i++) {
            assignedList += soundFilePaths[i];
            if (i < NUM_PADS - 1) {
                assignedList += ",";
            }
        }
        return assignedList;
    }


    // ---- New function to parse incoming Bluetooth commands ----
    void parseBluetoothCommand(String command) {

        Serial.println(command);

        command.trim();
        if (command.equalsIgnoreCase("list")) {
            String wavFiles = getWavFileList();
        // Serial.println("--- .wav files on SD card ---");
            Serial.println(wavFiles);
        // Serial.println("--- End of list ---");
        //   SerialBT.println("--- .wav files on SD card ---");
            SerialBT.println(wavFiles);
        // SerialBT.println("--- End of list ---");
            return;
        } else if (command.equalsIgnoreCase("config")) {
            // New command to get the currently assigned sounds
            String assignedSounds = getAssignedSounds();
        //  Serial.println("--- Assigned sounds from config file ---");
            Serial.println(assignedSounds);
        //  Serial.println("--- End of list ---");
        //  SerialBT.println("--- Assigned sounds from config file ---");
            SerialBT.println(assignedSounds);
        //  SerialBT.println("--- End of list ---");
            return;
        }

        // New logic to handle the 'assignPad' command
        int firstComma = command.indexOf(',');
        if (firstComma != -1) {
            String commandType = command.substring(0, firstComma);
            if (commandType.equalsIgnoreCase("assignPad")) {
                int secondComma = command.indexOf(',', firstComma + 1);
                if (secondComma != -1) {
                    String padString = command.substring(firstComma + 1, secondComma);
                    String waveFileName = command.substring(secondComma + 1);
                    int padNumber = padString.toInt();
                    updateConfigFile(padNumber, waveFileName.c_str());
                    return;
                }
            }
        }

        // Fallback for invalid commands
        Serial.println("Invalid command format. Use 'list', 'config', or 'assignPad,pad_number,filename.wav'");
        SerialBT.println("ERROR: Invalid command format. Use 'list', 'config', or 'assignPad,pad_number,filename.wav'");
    }

    // Function to read from a specific ADC channel
    int read_adc(int pin_idx) {
        if (pin_idx < 6) {
            // Use ADC1 for the first 6 pads
            return adc1_get_raw(adc1_channels[pin_idx]);
        } else {
            // Use ADC2 for the last 2 pads (pins 4 and 2)
            int raw_value = 0;
            // The ADC2 driver has a read function that takes the channel and a pointer to the value
            // Note: ADC2 can be used with WiFi, so it's a good practice to handle potential failures
            // We use pin_idx - 6 to get the correct index for the adc2_channels array.
            esp_err_t result = adc2_get_raw(adc2_channels[pin_idx - 6], ADC_WIDTH_BIT_12, &raw_value);
            if (result == ESP_OK) {
                return raw_value;
            } else {
                // Serial.printf("Error reading ADC2 channel %d: %s\n", adc2_channels[pin_idx - 6], esp_err_to_name(result));
                return 0; // Return 0 if there was a problem
            }
        }
    }

    // ---- Main setup function ----
    void setup() {
        Serial.begin(115200);

        // Initialize Bluetooth Serial
        SerialBT.begin("SmartPad-MBC");
        Serial.println("Bluetooth started. Ready to receive commands.");
        
        // Configure ADC1 for the 6 channels
        adc1_config_width(ADC_WIDTH_BIT_12);
        for (int i = 0; i < 6; i++) {
            adc1_config_channel_atten(adc1_channels[i], ADC_ATTEN_DB_11);
        }
        
        // ADC2 pins (4 and 2) do not require explicit channel configuration in setup,
        // as it is handled by adc2_get_raw.
        
        // SD Card initialization
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        if (!SD.begin(SD_CS)) {
            Serial.println("SD Card initialization failed!");
            while (1);
        }
        Serial.println("SD Card initialized.");
        
        // Load sound file paths from the config file
        loadConfig();

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
        // Check for Bluetooth commands
        if (SerialBT.available()) {
            String command = SerialBT.readStringUntil('\n');
            parseBluetoothCommand(command);
        }

        // Existing pad logic
        for (int i = 0; i < 2 ; i++) {
            // Read from the ADC using the new helper function
            int state = read_adc(i);

            if (state > HIT_THRESHOLD && (millis() - lastHitTime[i] > debounceDelay)) {
                Serial.printf("Pad %d hit! analog reading=%d\n", i, state);
                startPad(i, state); // Pass the analog reading as velocity
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
                // Apply the velocity gain to each sample before mixing
                mixedBuf[i] += (int16_t)((float)sample * playFiles[ch].velocityGain);
            }
        }

        // Apply global gain and clamp the final mixed audio
        for (size_t i = 0; i < BUFFER_LEN; i++) {
            int32_t mixedSample = (int32_t)(mixedBuf[i] * AUDIO_GAIN);

            // Clamp the value to 16-bit range to prevent clipping
            if (mixedSample > 32767) {
                mixedSample = 32767;
            } else if (mixedSample < -32768) {
                mixedSample = -32768;
            }
            mixedBuf[i] = (int16_t)mixedSample;
        }

        // Write the mixed audio to the I2S DAC
        size_t bytes_written;
        i2s_write(I2S_NUM_0, mixedBuf, BUFFER_SIZE_BYTES, &bytes_written, portMAX_DELAY);
    }

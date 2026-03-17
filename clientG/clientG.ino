    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <Fire_inferencing.h>   // Edge Impulse inference library

    // ===== WiFi Credentials (ESP32-CAM SoftAP) =====
    const char* ssid = "ESP32-CAM-LED";
    const char* password = "12345678";
    const char* serverUrl = "http://192.168.4.1/capture"; // ESP32-CAM default AP IP

    // ===== Image parameters from camera =====
    #define EI_CAMERA_RAW_FRAME_BUFFER_COLS 96
    #define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 96
    #define EI_CAMERA_FRAME_BYTE_SIZE (EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS)

    static uint8_t image_buffer[EI_CAMERA_FRAME_BYTE_SIZE];  // buffer to hold grayscale raw image

    // ===== Get image for Edge Impulse =====
    int get_image_data(size_t offset, size_t length, float *out_ptr) {
        for (size_t i = 0; i < length; i++) {
            // Protect against buffer overflow
            if (offset + i < EI_CAMERA_FRAME_BYTE_SIZE) {
                out_ptr[i] = image_buffer[offset + i] / 255.0f; // normalize 0–1
            } else {
                out_ptr[i] = 0.0f;
            }
        }
        return 0;
    }

    void setup() {
    Serial.begin(115200);
    delay(1000);

    // ===== Connect to ESP32-CAM WiFi AP =====
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Connected to ESP32-CAM");
    }

    void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {   // safer than raw 200
        WiFiClient *stream = http.getStreamPtr();
        size_t len = http.getSize();

        if (len == EI_CAMERA_FRAME_BYTE_SIZE) {
            stream->readBytes((char*)image_buffer, len);

            // Run inference
            signal_t signal;
            signal.total_length = EI_CAMERA_FRAME_BYTE_SIZE;
            signal.get_data = &get_image_data;

            ei_impulse_result_t result = {0};
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

            if (res != EI_IMPULSE_OK) {
            Serial.printf("❌ Failed to run classifier (%d)\n", res);
            } else {
            // Print classification results
            Serial.println("=== Classification results ===");
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                Serial.printf("%s: %.5f\n", 
                            result.classification[ix].label,
                            result.classification[ix].value);
            }

            // 🔥 Example: simple fire alarm trigger
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                if (strcmp(result.classification[ix].label, "fire") == 0 &&
                    result.classification[ix].value > 0.8f) {
                Serial.println("🚨 FIRE DETECTED! 🚨");
                // TODO: send command to CAM LED or buzzer
                }
            }
            }
        } else {
            Serial.printf("❌ Wrong image size: got %d, expected %d\n", (int)len, EI_CAMERA_FRAME_BYTE_SIZE);
        }
        } else {
        Serial.printf("❌ HTTP error: %d\n", httpCode);
        }

        http.end();
    } else {
        Serial.println("⚠️ WiFi not connected");
    }

    delay(2000); // wait before next inference
    }

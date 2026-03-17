/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Fire_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS   320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS   240
#define EI_CAMERA_FRAME_BYTE_SIZE         3


#define RESET_PIN 33  // Use an unused GPIO

static bool debug_nn = false;
static bool is_initialised = false;
uint8_t *snapshot_buf;

// Semaphore for camera access
SemaphoreHandle_t xCameraSemaphore;

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    // ULTRA LOW settings
    .xclk_freq_hz = 2000000,  // 2MHz - extremely low
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_GRAYSCALE,
    .frame_size = FRAMESIZE_QQVGA,  // 160x120 - smallest
    .jpeg_quality = 10,
    .fb_count = 1,  // Only 1 buffer to minimize DMA usage
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,  // Most conservative mode
};

bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println("Edge Impulse Inferencing Demo");
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());

    // Create semaphore for camera access
    xCameraSemaphore = xSemaphoreCreateMutex();
    
    // Reduce CPU frequency to absolute minimum
    setCpuFrequencyMhz(80);
    
    // Disable WiFi and Bluetooth to free up DMA resources
    WiFi.mode(WIFI_OFF);
    btStop();
    
    // Give some time for services to stop
    delay(1000);

    if (ei_camera_init() == false) {
        Serial.println("Failed to initialize Camera! Trying alternative approach...");
        
        // Try manual pin configuration as last resort
        camera_config.xclk_freq_hz = 1000000; // 1MHz
        if (ei_camera_init() == false) {
            Serial.println("Camera failed permanently. Hardware issue likely.");
            while(1) { delay(5000); }
        }
    }

    Serial.println("Starting inference in 5 seconds...");
    delay(5000);
}

void loop() {
    if (xSemaphoreTake(xCameraSemaphore, portMAX_DELAY) == pdTRUE) {
        // Check memory
        if (ESP.getFreeHeap() < 100000) {
            Serial.println("Low memory, skipping...");
            xSemaphoreGive(xCameraSemaphore);
            delay(1000);
            return;
        }

        snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
        if (!snapshot_buf) {
            Serial.println("Failed to allocate buffer");
            xSemaphoreGive(xCameraSemaphore);
            delay(1000);
            return;
        }

        // Try multiple capture attempts
        bool capture_success = false;
        for (int attempt = 0; attempt < 3; attempt++) {
            if (ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
                capture_success = true;
                break;
            }
            delay(100);
        }

        if (!capture_success) {
            Serial.println("All capture attempts failed");
            free(snapshot_buf);
            xSemaphoreGive(xCameraSemaphore);
            delay(2000);
            return;
        }

        ei::signal_t signal;
        signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
        signal.get_data = &ei_camera_get_data;

        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            Serial.printf("Classifier error: %d\n", err);
        } else {
            // Print results
            for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
                Serial.printf("%s: %.5f\n", ei_classifier_inferencing_categories[i], 
                              result.classification[i].value);
            }
        }

        free(snapshot_buf);
        xSemaphoreGive(xCameraSemaphore);
        
        // Long delay to ensure camera settles
        delay(3000);
    }
}

bool ei_camera_init(void) {
    if (is_initialised) return true;

    // Manual pin configuration to ensure proper setup
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);
    delay(100);

    // Initialize with extreme conservative settings
    for (int attempt = 0; attempt < 5; attempt++) {
        esp_err_t err = esp_camera_init(&camera_config);
        if (err == ESP_OK) {
            sensor_t *s = esp_camera_sensor_get();
            if (s) {
                // Minimal sensor configuration
                s->set_vflip(s, 1);
                s->set_hmirror(s, 1);
                s->set_gainceiling(s, GAINCEILING_2X);
                s->set_framesize(s, FRAMESIZE_QQVGA);
                
                is_initialised = true;
                Serial.println("Camera initialized with ultra-low settings");
                return true;
            }
        }
        
        Serial.printf("Camera init attempt %d failed: 0x%x\n", attempt + 1, err);
        delay(500);
        
        // Try even more conservative settings on retry
        if (attempt == 1) camera_config.xclk_freq_hz = 1000000;
        if (attempt == 2) camera_config.frame_size = FRAMESIZE_96X96;
    }
    
    return false;
}

void ei_camera_deinit(void) {
    esp_camera_deinit();
    is_initialised = false;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
      // Hardware reset camera
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    digitalWrite(RESET_PIN, HIGH);
    delay(100);

    if (!is_initialised) return false;

    // Stop any ongoing camera operations
    esp_camera_deinit();
    delay(50);
    
    // Reinitialize for this capture
    if (!ei_camera_init()) return false;

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("FB get failed");
        return false;
    }

    if (camera_config.pixel_format == PIXFORMAT_GRAYSCALE) {
        memcpy(out_buf, fb->buf, fb->len);
    } else {
        if (!fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, out_buf)) {
            esp_camera_fb_return(fb);
            return false;
        }
    }

    esp_camera_fb_return(fb);
    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        if (camera_config.pixel_format == PIXFORMAT_GRAYSCALE) {
            uint8_t gray = snapshot_buf[pixel_ix / 3]; // Grayscale is 1 byte per pixel
            out_ptr[out_ptr_ix] = (gray << 16) + (gray << 8) + gray;
            pixel_ix += 1;
        } else {
            out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + 
                                 (snapshot_buf[pixel_ix + 1] << 8) + 
                                 snapshot_buf[pixel_ix];
            pixel_ix += 3;
        }
        out_ptr_ix++;
        pixels_left--;
    }
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif
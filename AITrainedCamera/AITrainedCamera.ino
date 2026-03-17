    #include <Fire_inferencing.h>
    #include "edge-impulse-sdk/dsp/image/image.hpp"
    #include "esp_camera.h"

    // Camera model selection
    #define CAMERA_MODEL_AI_THINKER // Has PSRAM

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
    #else
    #error "Camera model not selected"
    #endif

    #define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
    #define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
    #define EI_CAMERA_FRAME_BYTE_SIZE 3

    static bool debug_nn = false;
    static bool is_initialised = false;

    // Allocate snapshot buffer globally once in PSRAM
    uint8_t *snapshot_buf = nullptr;

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

        .xclk_freq_hz = 8000000,      // slower XCLK to reduce DMA stress
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_QVGA, // lower resolution
        .jpeg_quality = 20,           // lower quality for smaller buffer
        .fb_count = 1,                // single buffer prevents DMA overflow
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST
    };

    bool ei_camera_init(void);
    void ei_camera_deinit(void);
    bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);

    void setup() {
        Serial.begin(115200);
        while (!Serial);

        Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

        Serial.println("Edge Impulse Inferencing Demo");

        if (!ei_camera_init()) {
            Serial.println("Failed to initialize Camera!");
            while (1) { delay(1000); }
        }

        // Allocate snapshot buffer once in PSRAM
        snapshot_buf = (uint8_t*)ps_malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS *
                                        EI_CAMERA_RAW_FRAME_BUFFER_ROWS *
                                        EI_CAMERA_FRAME_BYTE_SIZE);
        if (snapshot_buf == nullptr) {
            Serial.println("Failed to allocate snapshot buffer!");
            while (1) { delay(1000); }
        }

        Serial.println("Camera and buffer initialized");
        ei_sleep(2000);
    }

    void loop() {
        if (ESP.getFreeHeap() < 100000) {
            Serial.println("WARN: Low memory, skipping inference");
            delay(1000);
            return;
        }

        ei::signal_t signal;
        signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
        signal.get_data = &ei_camera_get_data;

        if (!ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
            Serial.println("Failed to capture image");
            delay(500);
            return;
        }

        ei_impulse_result_t result = {0};
        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            Serial.printf("ERR: Failed to run classifier (%d)\n", err);
            delay(500);
            return;
        }

        // Print predictions
        Serial.printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.):\n",
                    result.timing.dsp, result.timing.classification, result.timing.anomaly);

    #if EI_CLASSIFIER_OBJECT_DETECTION == 1
        for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
            auto bb = result.bounding_boxes[i];
            if (bb.value == 0) continue;
            Serial.printf("  %s (%f) [x:%u, y:%u, w:%u, h:%u]\n",
                        bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        }
    #else
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            Serial.printf("  %s: %.5f\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        }
    #endif

    #if EI_CLASSIFIER_HAS_ANOMALY
        Serial.printf("Anomaly prediction: %.3f\n", result.anomaly);
    #endif

        delay(1000); // spacing to prevent DMA overflow / overheating
    }

    // ----- Camera functions -----
    bool ei_camera_init(void) {
        if (is_initialised) return true;

        esp_err_t err = esp_camera_init(&camera_config);
        if (err != ESP_OK) {
            Serial.printf("Camera init failed with error 0x%x\n", err);
            return false;
        }

        sensor_t * s = esp_camera_sensor_get();
        if (!s) {
            Serial.println("Failed to get camera sensor");
            return false;
        }

        s->set_vflip(s, 1);
        s->set_hmirror(s, 1);

        is_initialised = true;
        Serial.println("Camera initialized successfully with optimized settings");
        return true;
    }

    void ei_camera_deinit(void) {
        esp_err_t err = esp_camera_deinit();
        if (err != ESP_OK) {
            Serial.println("Camera deinit failed");
            return;
        }
        is_initialised = false;
    }

    bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
        if (!is_initialised) return false;

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) return false;

        bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
        esp_camera_fb_return(fb);
        if (!converted) return false;

        if (img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS || img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS) {
            ei::image::processing::crop_and_interpolate_rgb888(
                out_buf,
                EI_CAMERA_RAW_FRAME_BUFFER_COLS,
                EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
                out_buf,
                img_width,
                img_height);
        }
        return true;
    }

    static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
        size_t pixel_ix = offset * 3;
        size_t out_ptr_ix = 0;
        while (length--) {
            out_ptr[out_ptr_ix++] = (snapshot_buf[pixel_ix + 2] << 16) +
                                    (snapshot_buf[pixel_ix + 1] << 8) +
                                    snapshot_buf[pixel_ix];
            pixel_ix += 3;
        }
        return 0;
    }

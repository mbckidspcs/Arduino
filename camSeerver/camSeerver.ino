#include <Arduino.h>
#include "esp_camera.h"

// ==== Camera Pin Config for AI Thinker ====
 #define CAMERA_MODEL_AI_THINKER
  #define PWDN_GPIO_NUM  32
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM  0
  #define SIOD_GPIO_NUM  26
  #define SIOC_GPIO_NUM  27

  #define Y9_GPIO_NUM    35
  #define Y8_GPIO_NUM    34
  #define Y7_GPIO_NUM    39
  #define Y6_GPIO_NUM    36
  #define Y5_GPIO_NUM    21
  #define Y4_GPIO_NUM    19
  #define Y3_GPIO_NUM    18
  #define Y2_GPIO_NUM    5
  #define VSYNC_GPIO_NUM 25
  #define HREF_GPIO_NUM  23
  #define PCLK_GPIO_NUM  22



// ==== Flash LED ====
#define LED_PIN 4

// ==== UART Config ====
#define CamSerial Serial1   // Use UART1
#define TX_PIN 14           // ESP32-CAM TX to Receiver RX
#define RX_PIN 15           // ESP32-CAM RX (not used)

#define FRAME_SIZE FRAMESIZE_QQVGA  // 160x120
#define BAUD_RATE  115200

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESP32-CAM Sender Ready");

  // Init LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Init UART1
  CamSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Init Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAME_SIZE;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    while (true);
  }
}

void loop() {
  // Turn LED ON
  digitalWrite(LED_PIN, HIGH);
  delay(100); // short delay to stabilize lighting

  // Capture frame
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    digitalWrite(LED_PIN, LOW);
    delay(2000);
    return;
  }

  // Send frame length first
  uint32_t len = fb->len;
  CamSerial.write((uint8_t*)&len, sizeof(len));

  // Send frame bytes
  CamSerial.write(fb->buf, len);
  CamSerial.flush();

  // Return frame buffer
  esp_camera_fb_return(fb);

  // Turn LED OFF
  digitalWrite(LED_PIN, LOW);

  Serial.printf("Sent %u bytes\n", len);
  delay(2000); // capture interval
}

/**
 * ESP32-CAM Stable Image Capture Server
 * Maximized stability to avoid DMA overflow
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// ===== WiFi settings =====
#define WIFI_SSID "Dialog 4G 713"
#define WIFI_PASS "3aF15bFF"

// ===== AI-Thinker ESP32-CAM pins =====
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

// ===== HTTP server =====
WebServer server(80);

// ===== WiFi reconnect =====
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 10000; // 10s

void connectWiFi();
void startCameraServer();

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("___ESP32-CAM MAX STABLE IMAGE SERVER___");

  // Connect to WiFi
  connectWiFi();

  // ===== Camera configuration =====
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000; // reduce DMA load
  config.pixel_format = PIXFORMAT_JPEG;

  // ===== Memory-safe settings =====
  #ifdef ESP32_HAS_PSRAM
    config.frame_size   = FRAMESIZE_QQVGA; // 160x120
    config.jpeg_quality = 25;
    config.fb_count     = 1;               // single buffer
    config.fb_location  = CAMERA_FB_IN_PSRAM;
  #else
    config.frame_size   = FRAMESIZE_QQVGA; // 160x120
    config.jpeg_quality = 25;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
  #endif

  // ===== Initialize camera =====
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed: 0x%x\n", err);
    while(true){ delay(1000); }
  }
  Serial.println("[OK] Camera initialized");

  // ===== Start HTTP server =====
  startCameraServer();
  Serial.println("[OK] Camera server running");
  Serial.print("[INFO] Access URL: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();

  // WiFi auto reconnect
  if (millis() - lastWifiCheck > wifiCheckInterval) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WARN] WiFi disconnected, reconnecting...");
      connectWiFi();
    }
  }
}

// ===== WiFi connect =====
void connectWiFi() {
  Serial.print("[INFO] Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected");
    Serial.print("[INFO] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] WiFi failed to connect");
  }
}

// ===== Camera HTTP server =====
void startCameraServer() {
  // HTML page with live image auto-refresh every 10s
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><body>";
    html += "<h1>ESP32-CAM Stable Image Capture Server</h1>";
    html += "<p>Image updates every 10 seconds for stability.</p>";
    html += "<img id='camImage' src='/capture' style='width:160px;height:auto;' />";
    html += "<script>"
            "setInterval(function(){"
            "document.getElementById('camImage').src='/capture?'+Date.now();"
            "},10000);" // refresh every 10s
            "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Capture JPEG endpoint
  server.on("/capture", HTTP_GET, []() {
    static unsigned long lastCapture = 0;
    if(millis() - lastCapture < 1000){ // min 1s between captures
        server.send(503, "text/plain", "Wait 1 second between captures");
        return;
    }
    lastCapture = millis();

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        server.send(500, "text/plain", "Camera capture failed");
        return;
    }

    // Minimal Serial output to reduce DMA stress
    server.sendHeader("Content-Type", "image/jpeg");
    server.sendHeader("Content-Length", String(fb->len));
    server.send(200);
    WiFiClient client = server.client();
    client.write(fb->buf, fb->len);
    esp_camera_fb_return(fb);
  });

  server.begin();
}

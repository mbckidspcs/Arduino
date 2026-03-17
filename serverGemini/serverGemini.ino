#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>

// Camera pin config for AI Thinker ESP32-CAM
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

#define LED_PIN 4
bool ledState = false;

const char* ssid = "ESP32-CAM-LED";
const char* password = "12345678";

WebServer server(80);

// Simple HTML control page
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Grayscale</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    button { padding: 10px 20px; margin: 10px; font-size: 16px; }
    a { display: block; margin: 10px; font-size: 18px; text-decoration: none; color: blue; }
  </style>
</head>
<body>
  <h2>🔥 ESP32-CAM Fire Detection</h2>
  <button onclick="toggleLED()">Toggle LED</button>
  <a href="/capture" target="_blank">📸 Download 96×96 Grayscale</a>
  <p id="status"></p>
  <script>
    function toggleLED() {
      fetch('/led', { method: 'POST' })
        .then(response => response.text())
        .then(text => document.getElementById('status').innerText = text)
        .catch(err => document.getElementById('status').innerText = "Error: " + err);
    }
  </script>
</body>
</html>
)rawliteral";

void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_96X96;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed");
    ESP.restart();
  } else {
    Serial.println("✅ Camera initialized");
  }
}

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleLED() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  server.send(200, "text/plain", ledState ? "LED ON" : "LED OFF");
}

void handleCapture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb || fb->width != 96 || fb->height != 96 || fb->format != PIXFORMAT_GRAYSCALE) {
    Serial.printf("❌ Capture failed: fb=%p, format=%d, size=%d\n", fb, fb ? fb->format : -1, fb ? fb->len : 0);
    server.send(500, "text/plain", "Capture failed");
    return;
  }

  server.sendHeader("Content-Disposition", "attachment; filename=grayscale.raw");
  server.sendHeader("Content-Type", "application/octet-stream");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.setContentLength(fb->len);
  server.send(200);
  server.sendContent((const char*)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.softAP(ssid, password);
  Serial.println("✅ AP started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  startCamera();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_POST, handleLED);
  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
  Serial.println("✅ Server started");
}

void loop() {
  server.handleClient();
}
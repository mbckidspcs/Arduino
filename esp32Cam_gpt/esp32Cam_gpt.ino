#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// ==== CHANGE HERE ====
const char* ssid     = "Dialog 4G 713";
const char* password = "3aF15bFF";

// ==== CAMERA MODEL (AI Thinker ESP32-CAM) ====
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

// ==== LED Flash (built-in) ====
#define LED_GPIO_NUM       4

WebServer server(80);
bool flashOn = false;

void handleRoot();
void handleJpg();
void handleJpgStream();
void handleFlashOn();
void handleFlashOff();
void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // ==== Camera Config ====
  camera_config_t config = {};
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size   = FRAMESIZE_UXGA;   // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count     = 2;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size   = FRAMESIZE_SVGA;   // 800x600
    config.jpeg_quality = 12;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  }

  // ==== Init Camera ====
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  // ==== LED Pin ====
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW);

  // ==== WiFi Connect ====
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // ==== Start Camera Web Server ====
  startCameraServer();
  Serial.println("Camera ready! Open the above IP in your browser");
}

void loop() {
  server.handleClient();
}

// ==== HANDLERS ====
void handleRoot() {
  String html = "<html><body><h1>ESP32-CAM Control</h1>";
  html += "<p><a href=\"/jpg\">📷 Take Photo</a></p>";
  html += "<p><a href=\"/stream\">🎥 Live Stream</a></p>";
  html += "<p><a href=\"/flashon\">💡 Flash ON</a></p>";
  html += "<p><a href=\"/flashoff\">💡 Flash OFF</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleJpg() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void handleJpgStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");
    esp_camera_fb_return(fb);
    if (!client.connected()) break;
  }
}

void handleFlashOn() {
  digitalWrite(LED_GPIO_NUM, HIGH);
  flashOn = true;
  server.send(200, "text/html", "<p>Flash ON ✅</p><a href=\"/\">Back</a>");
}

void handleFlashOff() {
  digitalWrite(LED_GPIO_NUM, LOW);
  flashOn = false;
  server.send(200, "text/html", "<p>Flash OFF ❌</p><a href=\"/\">Back</a>");
}

// ==== SERVER ROUTES ====
void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/jpg", HTTP_GET, handleJpg);
  server.on("/stream", HTTP_GET, handleJpgStream);
  server.on("/flashon", HTTP_GET, handleFlashOn);
  server.on("/flashoff", HTTP_GET, handleFlashOff);
  server.begin();
}

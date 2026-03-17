#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#define LED_PIN 4  // Onboard flash LED

// Camera pins for AI Thinker
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

const char* ssid = "ESP32-CAM-AP";
const char* password = "12345678";

WebServer server(80);
bool ledState = false;

#define PART_BOUNDARY "123456789000000000000987654321"

// ====== LED toggle handler ======
void handleToggleLED() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

// ====== Stream handler ======
void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=" PART_BOUNDARY "\r\n\r\n";
  client.print(response);

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;

    client.printf("--%s\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", PART_BOUNDARY, fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");
    esp_camera_fb_return(fb);

    if (!client.connected()) break;
    yield(); // allow background tasks
  }
}

// ====== Root page ======
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32-CAM Control</title>";
  html += "<style>";
  html += "body { font-family: Arial; text-align:center; padding: 20px; background: #f0f0f0; }";
  html += "h1 { color: #333; }";
  html += "button { font-size: 1.2em; padding: 15px 30px; margin: 10px; border: none; border-radius: 10px; cursor: pointer; }";
  html += "button:hover { opacity: 0.9; }";
  html += ".led { background: #ff4c4c; color: white; }";
  html += "@media(max-width:600px){ button { width: 80%; font-size: 1.5em; } }";
  html += "img { width: 100%; max-width: 320px; border: 2px solid #333; border-radius: 10px; }";
  html += "</style></head><body>";

  html += "<h1>ESP32-CAM Control Panel</h1>";
  html += "<p>LED is currently: <b>" + String(ledState ? "ON" : "OFF") + "</b></p>";
  html += "<a href='/toggle'><button class='led'>Toggle LED</button></a><br><br>";

  // Embedded live stream
  html += "<img src='/stream' alt='Live Stream'><br><br>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting ESP32-CAM...");

  // Camera config for smooth streaming
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  config.frame_size = FRAMESIZE_QVGA;  // small for higher FPS
  config.jpeg_quality = 12;            // balance quality/speed
  config.fb_count = 2;                 // double buffering

  if(esp_camera_init(&config) != ESP_OK){
    Serial.println("Camera init failed");
    return;
  }
  Serial.println("Camera ready");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggleLED);
  server.on("/stream", HTTP_GET, handleStream);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

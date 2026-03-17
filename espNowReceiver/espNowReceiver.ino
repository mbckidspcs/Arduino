#include <WiFi.h>
#include <esp_now.h>

typedef struct struct_message {
  bool buttonState;
} struct_message;

struct_message incomingData;

// New-style callback
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
  Serial.print("Received button state: ");
  Serial.println(incomingData.buttonState ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Callback handles everything
}

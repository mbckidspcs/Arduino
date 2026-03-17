    #include <esp_now.h>
    #include <WiFi.h>

    // Replace with your receiver's MAC address
    uint8_t receiverAddress[] = {0x40, 0x22, 0xD8, 0x05, 0x5F, 0x1C}; 


typedef struct struct_message {
  int id;
  char message[32];
} struct_message;

struct_message myData;

// New callback signature
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  myData.id = 1;
  strcpy(myData.message, "Hello ESP32!");

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent successfully");
  } else {
    Serial.println("Error sending the data");
  }

  delay(2000);
}
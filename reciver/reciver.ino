  #include <WiFi.h>
  #include <esp_now.h>

  typedef struct struct_message {
    int id;
    char message[32];
  } struct_message;

  struct_message incomingData;

  // New callback signature
  void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataPtr, int len) {
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
    Serial.print("Received from: ");
    for(int i = 0; i < 6; i++){
      Serial.print(info->src_addr[i], HEX);
      if(i < 5) Serial.print(":");
    }
    Serial.print(" -> ");
    Serial.println(incomingData.message);
  }

  void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Register the receive callback (correct signature)
    esp_now_register_recv_cb(OnDataRecv);
  }

  void loop() {
    // Reception happens automatically in the callback
  }

  #include <WiFi.h>
  #include <esp_now.h>

  const int buttonPin = 4;

  typedef struct struct_message {
    bool buttonState;
  } struct_message;

  struct_message myData;

  // New send callback
  void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
    Serial.print("Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  }

  void setup() {
    Serial.begin(115200);
    pinMode(buttonPin, INPUT_PULLUP);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    esp_now_register_send_cb(OnDataSent);

    // Broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memset(peerInfo.peer_addr, 0xFF, 6); // broadcast
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  void loop() {
    myData.buttonState = digitalRead(buttonPin) == LOW;
    esp_err_t result = esp_now_send((uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF", (uint8_t*)&myData, sizeof(myData));
    if (result != ESP_OK) {
      Serial.println("Send failed");
    }
    delay(200);
  }

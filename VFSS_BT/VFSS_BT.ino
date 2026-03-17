  #include "BluetoothSerial.h"
  #include <Preferences.h>

  BluetoothSerial SerialBT;
  Preferences preferences;

  String receivedText = "";
  String savedText = "";
  String dataRequested = "F";

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("VFSS BT"); // ESP32 BT device name
    Serial.println("✅ ESP32 Bluetooth ready. Waiting for text...");

    preferences.begin("storage", false);
    preferences.putString("bt_text","Name:Test|Message:755991839|Mobile:755991832");

    // Show previously saved value
    if (preferences.isKey("bt_text")) {
      String saved = preferences.getString("bt_text");
      Serial.print("📂 Last saved text: ");
      Serial.println(saved);
    }
  }

  void loop() {

    while (SerialBT.available()) {
      char c = SerialBT.read();

      // Text ends when a newline is received
      if (c == '\n' || c == '\r') {
        if (receivedText.length() > 0) {
          Serial.print("📩 Received: ");
          Serial.println(receivedText);

          if(receivedText.startsWith("GET_DATA")){

            dataRequested = "T";

          }else{
          // Save to flash memory
          preferences.putString("bt_text", receivedText);
          Serial.println("💾 Saved to memory!");

          receivedText = ""; // Reset buffer
          }
        }
      } else {
        receivedText += c;
      }

    }

    if (SerialBT.hasClient() && dataRequested == "T") {

      Serial.println("🔵 Client connected. Sending saved text...");
      SerialBT.println("📤 From ESP32 memory:");

    // Retrieve previously saved text
    if (preferences.isKey("bt_text")) {
      savedText = preferences.getString("bt_text");
      Serial.print("📂 Loaded from memory: ");
      Serial.println(savedText);

    } else {
      savedText = "No saved data.";
      Serial.println("⚠️ No text found in memory.");
    }


      SerialBT.println(savedText);
      dataRequested = "";
      delay(10000); // Avoid sending repeatedly — wait 10 sec before next send

    } else {
      Serial.println("🔴 No client connected...");
    }

    delay(1000);
    Serial.println(preferences.getString("bt_text"));
  }

  #include <BluetoothSerial.h>
  #include <Preferences.h>

  BluetoothSerial SerialBT;
  Preferences preferences;

  // Variables
  String VehicleNumber;
  String MobileNumber1;
  String MobileNumber2;

  void loadDataFromMemory() {
    VehicleNumber = preferences.getString("VehicleNumber", "ABC-0000");  
    MobileNumber1 = preferences.getString("Mobile1", "0000000000");
    MobileNumber2 = preferences.getString("Mobile2", "0000000000");
  }

  void saveDataToMemory() {
    preferences.putString("VehicleNumber", VehicleNumber);
    preferences.putString("Mobile1", MobileNumber1);
    preferences.putString("Mobile2", MobileNumber2);
  }

  void sendFormattedData() {
    String data = "Name:" + VehicleNumber + 
                  "|Message:" + MobileNumber1 + 
                  "|Mobile:" + MobileNumber2;
    SerialBT.println(data);
  }

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32_Vehicle");  // Bluetooth name
    preferences.begin("vehicleData", false);

    // Load saved data
    loadDataFromMemory();

    Serial.println("ESP32 Bluetooth Started. Waiting for connection...");
  }

  void loop() {
    if (SerialBT.hasClient()) {
      if (SerialBT.available()) {
        String incoming = SerialBT.readStringUntil('\n');
        incoming.trim();
        Serial.println("Received: " + incoming);

        if (incoming.startsWith("GET_DATA")) {
          // Mobile app requested data
          sendFormattedData();

        } else if (incoming.startsWith("UPDATE:")) {
          // Example: UPDATE:Name:ABC123|Message:0771234567|Mobile:0712345678
          incoming.replace("UPDATE:", "");
          int sep1 = incoming.indexOf("|");
          int sep2 = incoming.lastIndexOf("|");

          if (sep1 > 0 && sep2 > sep1) {
            String namePart = incoming.substring(0, sep1);
            String msgPart = incoming.substring(sep1 + 1, sep2);
            String mobPart = incoming.substring(sep2 + 1);

            // Parse key:value
            if (namePart.startsWith("Name:")) {
              VehicleNumber = namePart.substring(5);
            }
            if (msgPart.startsWith("Message:")) {
              MobileNumber1 = msgPart.substring(8);
            }
            if (mobPart.startsWith("Mobile:")) {
              MobileNumber2 = mobPart.substring(7);
            }

            saveDataToMemory();  // Save updated values
            Serial.println("Data updated & saved!");

            // Send back confirmation
            sendFormattedData();
          }
        }
      }
    }
  }

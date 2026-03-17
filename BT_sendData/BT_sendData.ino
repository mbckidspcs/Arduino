#include <BluetoothSerial.h>
#include <EEPROM.h>

BluetoothSerial SerialBT;

#define EEPROM_SIZE 512

// Define max lengths for fields
#define MAX_NAME_LEN 64
#define MAX_MSG_LEN 128
#define MAX_MOBILE_LEN 20

struct StoredData {
  char name[MAX_NAME_LEN];
  char message[MAX_MSG_LEN];
  char mobile[MAX_MOBILE_LEN];
};

StoredData data;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Load stored data from EEPROM
  EEPROM.get(0, data);

  // Ensure strings are null-terminated
  data.name[MAX_NAME_LEN - 1] = '\0';
  data.message[MAX_MSG_LEN - 1] = '\0';
  data.mobile[MAX_MOBILE_LEN - 1] = '\0';

  SerialBT.begin("ESP32_Bluetooth"); // Set device name
  Serial.println("Bluetooth started. Waiting for connections...");
}

void loop() {
  if (SerialBT.hasClient()) {
    // Send current stored data on connect (once)
    static bool sentData = false;
    if (!sentData) {
      sendStoredData();
      sentData = true;
    }

    if (SerialBT.available()) {
      String incoming = SerialBT.readStringUntil('\n');
      incoming.trim();

      if (incoming.length() > 0) {
        // Buffer to store whole message (Name, Message, Mobile)
        static String buffer = "";
        buffer += incoming + "\n";

        // If message complete? We expect three lines, check by count
        int count = 0;
        for (int i = 0; i < buffer.length(); i++) {
          if (buffer.charAt(i) == '\n') count++;
        }

        if (count >= 3) {
          parseAndStore(buffer);
          buffer = "";
          // Optionally confirm receipt
          SerialBT.println("Data updated.");
        }
      }
    }
  } else {
    // Reset sentData flag when no client connected
    static bool sentData = false;
    if (sentData) sentData = false;
  }

  delay(20);
}

void sendStoredData() {
  SerialBT.print("Name: ");
  SerialBT.println(data.name);
  SerialBT.print("Message: ");
  SerialBT.println(data.message);
  SerialBT.print("Mobile: ");
  SerialBT.println(data.mobile);
}

void parseAndStore(String buffer) {
  // Parse string lines
  String nameVal = "";
  String messageVal = "";
  String mobileVal = "";

  int start = 0;
  for (int i = 0; i < 3; i++) {
    int end = buffer.indexOf('\n', start);
    if (end == -1) break;
    String line = buffer.substring(start, end);
    int sep = line.indexOf(':');
    if (sep > 0) {
      String key = line.substring(0, sep);
      String val = line.substring(sep + 1);
      val.trim();

      if (key == "Name") nameVal = val;
      else if (key == "Message") messageVal = val;
      else if (key == "Mobile") mobileVal = val;
    }
    start = end + 1;
  }

  // Copy to struct with safety
  nameVal.toCharArray(data.name, MAX_NAME_LEN);
  messageVal.toCharArray(data.message, MAX_MSG_LEN);
  mobileVal.toCharArray(data.mobile, MAX_MOBILE_LEN);

  // Save to EEPROM
  EEPROM.put(0, data);
  EEPROM.commit();

  Serial.println("Data stored:");
  Serial.println(data.name);
  Serial.println(data.message);
  Serial.println(data.mobile);
}

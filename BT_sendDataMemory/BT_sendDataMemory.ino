    #include <BluetoothSerial.h>
    #include <EEPROM.h>

    BluetoothSerial SerialBT;

    #define EEPROM_SIZE 512

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
      data.name[MAX_NAME_LEN - 1] = '\0';
      data.message[MAX_MSG_LEN - 1] = '\0';
      data.mobile[MAX_MOBILE_LEN - 1] = '\0';

      SerialBT.begin("ESP32_BT_Device");
      Serial.println("Bluetooth device started.");
    }

    String incomingBuffer = "";
    bool receivingUpdate = false;

    void loop() {
      if (SerialBT.hasClient()) {
        while (SerialBT.available()) {
          char c = SerialBT.read();
          incomingBuffer += c;

          if (!receivingUpdate && incomingBuffer.endsWith("\n")) {
            String command = incomingBuffer;
            command.trim();
            if (command == "GET_DATA") {
              sendStoredData();
              incomingBuffer = "";
            } else if (command.startsWith("UPDATE:")) {
              receivingUpdate = true;
              incomingBuffer = command.substring(7); // remove "UPDATE:"
              // wait to receive full data until newline
            } else {
              incomingBuffer = "";
            }
          } else if (receivingUpdate && incomingBuffer.endsWith("\n")) {
            // Parse the update string: "Name:xxx|Message:yyy|Mobile:zzz"
            parseAndStore(incomingBuffer);
            SerialBT.println("Data updated.");
            incomingBuffer = "";
            receivingUpdate = false;
          }
        }
      }
      delay(20);
    }

    void sendStoredData() {
      String response = "Name: " + String(data.name) + "|Message: " + String(data.message) + "|Mobile: " + String(data.mobile) + "\n";
      SerialBT.print(response);
      Serial.println("Sent data: " + response);
    }

    void parseAndStore(String buffer) {
      String nameVal = "";
      String messageVal = "";
      String mobileVal = "";

      int start = 0;
      while (true) {
        int sep = buffer.indexOf('|', start);
        String token;
        if (sep == -1) {
          token = buffer.substring(start);
        } else {
          token = buffer.substring(start, sep);
        }
        start = sep + 1;

        int colon = token.indexOf(':');
        if (colon > 0) {
          String key = token.substring(0, colon);
          String val = token.substring(colon + 1);
          val.trim();

          if (key == "Name") nameVal = val;
          else if (key == "Message") messageVal = val;
          else if (key == "Mobile") mobileVal = val;
        }

        if (sep == -1) break;
      }

      nameVal.toCharArray(data.name, MAX_NAME_LEN);
      messageVal.toCharArray(data.message, MAX_MSG_LEN);
      mobileVal.toCharArray(data.mobile, MAX_MOBILE_LEN);

      EEPROM.put(0, data);
      EEPROM.commit();

      Serial.println("Stored data:");
      Serial.println("Name: " + String(data.name));
      Serial.println("Message: " + String(data.message));
      Serial.println("Mobile: " + String(data.mobile));
    }

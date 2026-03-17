

  #include <HardwareSerial.h>

  HardwareSerial sim900(1);  // Use UART1 on ESP32

  void setup() {
    // Start debug serial
    Serial.begin(115200);

    // Start SIM900 serial (UART1)
    sim900.begin(9600, SERIAL_8N1, 27, 26); // RX=27, TX=26
    delay(1000);

    Serial.println("Initializing SIM900A...");

    sendCommand("AT");          // Test module
    sendCommand("AT+CSQ");      // Signal quality
    sendCommand("AT+CREG?");    // Network registration
    sendCommand("AT+CMGF=1");   // Set SMS text mode

    // Send SMS
    sendSMS("0703927827", "Hello from ESP32 + SIM900A!");
  }

  void loop() {
    // Print any response from SIM900A
    if (sim900.available()) {
      Serial.write(sim900.read());
    }

    
  }

  void sendCommand(const char *cmd) {
    sim900.println(cmd);
    delay(1000);
    while (sim900.available()) {
      Serial.write(sim900.read());
    }
  }

  void sendSMS(const char *number, const char *message) {
    Serial.println("Sending SMS...");

    sim900.print("AT+CMGS=\"");
    sim900.print(number);
    sim900.println("\"");
    delay(1000);

    sim900.print(message);
    delay(500);

    sim900.write(26); // Ctrl+Z to send
    delay(5000);

    Serial.println("SMS Sent!");
  }

#include <HardwareSerial.h>

HardwareSerial sim900(2); // Use UART2 (GPIO16=RX, GPIO17=TX)

void setup() {
  Serial.begin(115200);       // For debugging
  sim900.begin(9600, SERIAL_8N1, 16, 17); // RX, TX

  Serial.println("Initializing GSM...");

  delay(1000);
  sendCommand("AT");          // Check module
  sendCommand("AT+CMGF=1");   // Set SMS text mode
  sendCommand("AT+CSCS=\"GSM\""); // Set char set
 // sendSMS("+94771234567", "Hello from ESP32 and SIM900A!"); // Change to your number
}

void loop() {
  // Nothing here
  sendSMS("0786014252", "Vehicle Fire Security System(VFSS) - Testing SMS");
  delay(3000);
}

void sendCommand(const String& cmd) {
  sim900.println(cmd);
  delay(500);
  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

void sendSMS(const String& number, const String& message) {
  sim900.print("AT+CMGS=\"");
  sim900.print(number);
  sim900.println("\"");
  delay(1000);
  sim900.print(message);
  sim900.write(26); // CTRL+Z to send
  delay(5000);

  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

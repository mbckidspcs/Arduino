#include <HardwareSerial.h>

HardwareSerial sim900(1); // Use UART1

void setup() {
  Serial.begin(115200); // Debugging
  sim900.begin(9600, SERIAL_8N1, 27, 26); // RX=26, TX=27

  Serial.println("Initializing SIM900A...");
  delay(1000);

  // Set SMS mode to text
  sim900.println("AT+CMGF=1");
  delay(1000);

  // Enable caller ID notification
  sim900.println("AT+CLIP=1");
  delay(1000);

  // Send SMS
  sim900.println("AT+CMGS=\"0703927827\""); // Replace with your number
  delay(1000);
  sim900.print("Hello from ESP32 + SIM900A! 11111");
  delay(500);
  sim900.write(26); // CTRL+Z to send
  Serial.println("SMS Sent!");
}

void loop() {
  if (sim900.available()) {
    String response = sim900.readStringUntil('\n');
    response.trim();

    if (response.length() > 0) {
      Serial.println("SIM900A: " + response);

      // Incoming call alert
      if (response.startsWith("+CLIP:")) {
        Serial.println("📞 Incoming Call Detected!");
      }

      // New SMS alert
      if (response.startsWith("+CMTI:")) {
        int index = response.substring(response.lastIndexOf(',') + 1).toInt();
        Serial.println("📩 New SMS Received at index: " + String(index));

        // Read the SMS
        sim900.println("AT+CMGR=" + String(index));
        delay(1000);
      }

      // SMS content
      if (response.startsWith("+CMGR:")) {
        Serial.println("📨 SMS Content:");
        delay(500);
        while (sim900.available()) {
          String sms = sim900.readStringUntil('\n');
          sms.trim();
          if (sms.length() > 0) Serial.println(sms);
        }
      }
    }
  }
}
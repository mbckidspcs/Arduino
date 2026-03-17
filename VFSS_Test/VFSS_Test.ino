// Use HardwareSerial port 2 for SIM900
HardwareSerial sim900(2); // Use UART2

// Pin definitions
const int flamePin = 34;                // Flame sensor analog or digital
const int flameThreshold = 400;         // Adjust based on sensor reading
bool fireDetected = false;

// Your mobile number (international format)
String phoneNumber = "+94755991832";    // Replace with your number
String message = " Fire detected by ESP32!";

void setup() {
  Serial.begin(115200);                 // Serial Monitor
  sim900.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17 (to SIM900)
  delay(3000);

  Serial.println("System Initialized.");
  pinMode(flamePin, INPUT);

  // Optional: wait for GSM module to respond
  sim900.println("AT");
  delay(1000);
  flushSIM900();
}

void loop() {
  int flameValue = analogRead(flamePin);
  Serial.print("Flame Sensor Value: ");
  Serial.println(flameValue);

  if (flameValue < flameThreshold && !fireDetected) {
    Serial.println(" Fire Detected! Sending SMS...");
    sendSMS(phoneNumber, message);
    fireDetected = true;
  }

  if (flameValue >= flameThreshold) {
    fireDetected = false;
  }

  delay(1000);
}

void sendSMS(String number, String text) {
  sim900.println("AT+CMGF=1"); // Set SMS to text mode
  delay(1000);
  sim900.println("AT+CMGS=\"" + number + "\"");
  delay(1000);
  sim900.print(text);
  delay(500);
  sim900.write(26); // ASCII code of CTRL+Z to send SMS
  delay(5000);
  Serial.println(" SMS Sent!");
}

void flushSIM900() {
  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

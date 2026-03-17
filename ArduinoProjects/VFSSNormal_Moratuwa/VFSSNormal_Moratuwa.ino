#include <TinyGPS++.h>

  // Pin Definitions
  const int flamePins[3] = {32, 33, 25};     // Flame sensors
  const int relayPins[3] = {18, 19, 21};     // Relays for each flame sensor
  const int relay4Pin = 22;                  // Relay for smoke sensor
  const int smokePin = 36;                   // Analog pin for MQ-135 smoke
  const int buzzerPin = 23;                  // Buzzer

  // Threshold for smoke detection
  const int smokeThreshold = 1200; // Adjust based on your environment

  // GPS & GSM Setup
  HardwareSerial gpsSerial(1);  // UART1
  HardwareSerial gsmSerial(2);  // UART2
  TinyGPSPlus gps;

  // Phone numbers to send SMS
  String phoneNumber1 = "+94755991832"; // Replace with actual
  String phoneNumber2 = "+94766144250"; // Replace with actual

  // Names of areas corresponding to flame sensors
  String flameAreas[3] = {"Engine Room", "Diesel Tank", "Dikki"};
  String smokeArea = "Cabin";

  void setup() {
    Serial.begin(115200);

    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // GPS RX=16, TX=17
    gsmSerial.begin(9600, SERIAL_8N1, 26, 27);  // GSM RX=26, TX=27

    // Setup pins
    for (int i = 0; i < 3; i++) {
      pinMode(flamePins[i], INPUT);
      pinMode(relayPins[i], OUTPUT);
      digitalWrite(relayPins[i], HIGH);  // Relays OFF (active LOW)
    }

    pinMode(relay4Pin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    digitalWrite(relay4Pin, HIGH);  // OFF
    digitalWrite(buzzerPin, LOW);   // Buzzer OFF

    // Initialize GSM
    delay(1000);
    gsmSerial.println("AT");
    delay(1000);
    gsmSerial.println("AT+CMGF=1"); // SMS Text Mode
    delay(1000);
  }

  void loop() {
    // Read GPS data continuously
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }

    // Check flame sensors
    for (int i = 0; i < 3; i++) {
      if (digitalRead(flamePins[i]) == HIGH) {  // Fire detected
        digitalWrite(relayPins[i], LOW);  // Turn ON relay
        Serial.print("Fire detected in ");
        Serial.println(flameAreas[i]);

        triggerAlert(" Fire detected in " + flameAreas[i]);

        delay(2000);
        digitalWrite(relayPins[i], HIGH); // Turn OFF relay after alert
      }
    }

    // Check smoke sensor
    int smokeValue = analogRead(smokePin);
    Serial.print("Smoke Value: ");
    Serial.println(smokeValue);

    if (smokeValue > smokeThreshold) {
      digitalWrite(relay4Pin, LOW);   // Relay 4 ON
      digitalWrite(buzzerPin, HIGH);  // Buzzer ON

      triggerAlert(" Smoke detected in " + smokeArea + "! Value: " + String(smokeValue));

      delay(3000);

      digitalWrite(relay4Pin, HIGH);  // Relay 4 OFF
      digitalWrite(buzzerPin, LOW);   // Buzzer OFF
    }

    delay(500);
  }

  void triggerAlert(String reason) {
    String location = getGPSLocation();
    String message = reason + "\nLocation: " + location;

    sendSMS(message, phoneNumber1); 
    //delay(2000);
    sendSMS(message, phoneNumber2);
  }

  String getGPSLocation() {
    if (gps.location.isValid()) {
      return "https://maps.google.com/?q=" + 
            String(gps.location.lat(), 6) + "," + 
            String(gps.location.lng(), 6);
    } else {
      return "GPS not fixed.";
    }
  }

  void sendSMS(String msg, String phoneNumber) {
    Serial.print("Sending SMS to ");
    Serial.println(phoneNumber);
    Serial.println(msg);

    gsmSerial.println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    gsmSerial.print(msg);
    delay(1000);
    gsmSerial.write(26); // Ctrl+Z to send
    delay(2000);
  }
  
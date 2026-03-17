#include <TinyGPS++.h>
#include "BluetoothSerial.h"
#include <Preferences.h>

BluetoothSerial SerialBT;
Preferences preferences;

String receivedText = "";
String savedText = "";
String dataRequested = "F";

// Pin Definitions
const int flamePins[3] = { 32, 33, 25 };  
// Flame sensors
const int relayPins[3] = { 18, 19, 21 };  // Relays for each flame sensor
const int relay4Pin = 22;                 // Relay for smoke
const int smokePin = 36;                  // Analog pin for MQ-135
const int buzzerPin = 23;                 // Buzzer

// Threshold for smoke detection
const int smokeThreshold = 1000;  // Adjust based on your environment
String sensorArea[3] = { "in the Engine Room", "near Diesel Tank", "in the Dikki" };

// GPS & GSM Setup
HardwareSerial gpsSerial(1);  // UART1
HardwareSerial gsmSerial(2);  // UART2
TinyGPSPlus gps;

// Phone numbers to send SMS
String MobileNumber1 = "";  // Replace with actual
String MobileNumber2 = "";  // Replace with actual
String VehicleNumber = "";

void setup() {
  Serial.begin(115200);

  SerialBT.begin("VFSS");  // Bluetooth name
  preferences.begin("vehicleData", false);

  // Load saved data
  loadDataFromMemory();

  Serial.println("ESP32 System Started. Waiting for connections...");

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // GPS (RX=16, TX=17)
  gsmSerial.begin(9600, SERIAL_8N1, 26, 27);  // GSM (RX=26, TX=27)

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
  gsmSerial.println("AT+CMGF=1");  // SMS Text Mode
  delay(1000);
}

void loop() {
  // Read GPS data continuously
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  handleBT();

  // Check flame sensors (Active HIGH = fire detected)
  for (int i = 0; i < 3; i++) {
    if (digitalRead(flamePins[i]) == HIGH) {
      digitalWrite(relayPins[i], LOW);  // Turn ON relay
      Serial.print(" Fire detected on Sensor ");
      Serial.println(i + 1);

      triggerAlert("A fire has been detected in Vehicle " + VehicleNumber + " " + sensorArea[i]);

      delay(2000);
      digitalWrite(relayPins[i], HIGH);  // Turn OFF relay after alert
    }
  }

  // Check smoke sensor
  int smokeValue = analogRead(smokePin);
  //Serial.print("Smoke Value: ");
  //Serial.println(smokeValue);

  if (smokeValue > smokeThreshold) {
    digitalWrite(relay4Pin, LOW);   // Relay 4 ON
    digitalWrite(buzzerPin, HIGH);  // Buzzer ON

    triggerAlert("A smoke has been detected in Vehicle " + VehicleNumber + " Cabin");

    delay(3000);

    digitalWrite(relay4Pin, HIGH);  // Relay 4 OFF
    digitalWrite(buzzerPin, LOW);   // Buzzer OFF
  }

  delay(500);
}

void triggerAlert(String reason) {
  String location = getGPSLocation();
  String message = reason + "\nLocation: " + location;

  sendSMS(message, MobileNumber1);
  delay(5000);
  sendSMS(message, MobileNumber2);
}

String getGPSLocation() {
  if (gps.location.isValid()) {
    return "https://maps.google.com/?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  } else {
    return "GPS not fixed.";
  }
}

// ✅ Improved SMS sending with BT handling
void sendSMS(String msg, String phoneNumber) {
  Serial.print("Sending SMS to ");
  Serial.println(phoneNumber);
  Serial.println(msg);

  gsmSerial.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  gsmSerial.print(msg);
  delay(1000);
  gsmSerial.write(26);  // Ctrl+Z to send
  delay(2000);
}


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
  String data = "Name:" + VehicleNumber + "|Message:" + MobileNumber1 + "|Mobile:" + MobileNumber2;
  SerialBT.println(data);
}

void handleBT() {
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
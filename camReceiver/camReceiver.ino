#include <Arduino.h>
#define RX_PIN 16
#define TX_PIN 17
HardwareSerial CamSerial(2);
NEW SKETCH

void setup() {
  Serial.begin(115200);
  CamSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
}
void loop() {
  if (CamSerial.available()) {
    Serial.println(CamSerial.readString());
  }
}

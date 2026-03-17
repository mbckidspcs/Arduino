#include <ESP32Servo.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

BluetoothSerial SerialBT;
Servo gunServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const float g = 9.81;
float angle = 45.0;
float speed = 5.0;
float initialHeight = 0.3;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Projectile");

  gunServo.setPeriodHertz(50);
  gunServo.attach(5);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Projectile Ready");
  delay(1000);
  lcd.clear();
}

float calculateRange(float v, float theta, float h) {
  float rad = theta * PI / 180.0;
  float v_x = v * cos(rad);
  float v_y = v * sin(rad);
  float t = (v_y + sqrt(v_y * v_y + 2 * g * h)) / g;
  return v_x * t;
}

void loop() {
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');  // wait for newline
    data.trim(); // remove whitespace or \r

    Serial.print("Received: ");
    Serial.println(data);

    // Expect format: A:45,H:0.3,S:5
    int aIndex = data.indexOf("A:");
    int hIndex = data.indexOf("H:");
    int sIndex = data.indexOf("S:");

    if (aIndex != -1) {
      int comma = data.indexOf(',', aIndex);
      angle = data.substring(aIndex + 2, (comma != -1 ? comma : data.length())).toFloat();
      angle = constrain(angle, 0, 180);
      gunServo.write(angle);
    }
    if (hIndex != -1) {
      int comma = data.indexOf(',', hIndex);
      initialHeight = data.substring(hIndex + 2, (comma != -1 ? comma : data.length())).toFloat();
    }
    if (sIndex != -1) {
      int comma = data.indexOf(',', sIndex);
      speed = data.substring(sIndex + 2, (comma != -1 ? comma : data.length())).toFloat();
    }

    float distance = calculateRange(speed, angle, initialHeight);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Angle:");
    lcd.print(angle, 0);
    lcd.print((char)223);
    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distance, 2);
    lcd.print(" m");

    // Send back to app
    String response = "D:" + String(distance, 2) + "\n";
    SerialBT.print(response);
  }
}

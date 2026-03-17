#include <Arduino.h>
#include <BluetoothSerial.h>   // Bluetooth Classic SPP
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ====== CONFIG ======
static const int SERVO_PIN = 18;        // Servo pin
static const int LCD_ADDR  = 0x27;      // I2C LCD address (try 0x3F if nothing shows)
static const float g = 9.80665f;        // Gravity (m/s^2)
static const float bulletSpeed = 50.0f; // Hardcoded bullet speed (m/s)
static const float angleMinDeg = 0.0f;
static const float angleMaxDeg = 89.0f; // Avoid 90° → zero range
static const float servoOffset = 0.0f;  // Adjust if mechanical offset exists

// ====== GLOBALS ======
BluetoothSerial SerialBT;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Servo gunServo;

String rxBuf;
float currentAngleDeg = 0.0f;

// ----- Helpers -----
float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }
float deg2rad(float d) { return d * PI / 180.0f; }

float computeRange(float v, float angleDeg) {
  // R = v^2 * sin(2θ) / g
  float rad = deg2rad(angleDeg);
  float s   = sinf(2.0f * rad);
  float R   = (v * v) * s / g; // meters
  if (R < 0) R = 0;
  return R;
}

void moveServoToAngle(float angleDeg) {
  float servoAngle = clampf(angleDeg + servoOffset, 0.0f, 180.0f);
  gunServo.write((int)roundf(servoAngle));
}

void updateLCD(float angleDeg, float rangeM) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Angle:");
  lcd.print(angleDeg, 1);
  lcd.print(" deg");

  lcd.setCursor(0, 1);
  lcd.print("Range:");
  lcd.print(rangeM, 2);
  lcd.print(" m");
}

void sendStatusBT(float angleDeg, float rangeM) {
  SerialBT.print("ANGLE=");
  SerialBT.print(angleDeg, 1);
  SerialBT.print(",RANGE=");
  SerialBT.print(rangeM, 2);
  SerialBT.print("\n");
}

void applyAngle(float angleDeg) {
  currentAngleDeg = clampf(angleDeg, angleMinDeg, angleMaxDeg);
  moveServoToAngle(currentAngleDeg);
  float R = computeRange(bulletSpeed, currentAngleDeg);
  updateLCD(currentAngleDeg, R);
  sendStatusBT(currentAngleDeg, R);
}

// ----- Setup -----
void setup() {
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SpringGun Ready");
  lcd.setCursor(0, 1);
  lcd.print("BT: SpringGunESP");

  // Servo (50Hz)
  gunServo.setPeriodHertz(50);
  gunServo.attach(SERVO_PIN, 500, 2400);
  moveServoToAngle(0);

  // Bluetooth SPP
  if (!SerialBT.begin("SpringGunESP32")) {
    Serial.println("BT init failed!");
  } else {
    Serial.println("BT ready. Pair & connect.");
  }

  // Start default angle
  applyAngle(0);
}

// ----- Loop -----
void loop() {
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\r') continue;
    if (c == '\n') {
      String line = rxBuf; rxBuf = "";
      line.trim();
      if (line.length() == 0) continue;

      // Parse command
      if (line.startsWith("A:") || line.startsWith("a:")) {
        float angle = line.substring(2).toFloat();
        applyAngle(angle);
      } else if (line.equalsIgnoreCase("Q")) {
        float R = computeRange(bulletSpeed, currentAngleDeg);
        sendStatusBT(currentAngleDeg, R);
      } else {
        SerialBT.print("ERR:Unknown cmd\n");
      }
    } else {
      rxBuf += c;
      if (rxBuf.length() > 64) rxBuf = ""; // overflow guard
    }
  }
}

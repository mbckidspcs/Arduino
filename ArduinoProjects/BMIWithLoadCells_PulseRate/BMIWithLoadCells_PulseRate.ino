#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// ------------------ HX711 ------------------
#define DT 33
#define SCK 32
HX711 scale;

// HX711 calibration
float calibration_factor = 22.112848;
long offset = 135632;

// ------------------ Ultrasonic ------------------
#define TRIG_PIN 4
#define ECHO_PIN 5
const float SENSOR_HEIGHT_M = 2.10; // sensor mounting height in meters

// ------------------ Pulse Sensor ------------------
#define PULSE_PIN 34  // connect HW-827 output to GPIO34 (ADC pin)
int pulseValue = 0;
int bpm = 0;
bool fingerDetected = false;

// ------------------ LCD ------------------
LiquidCrystal_I2C lcd(0x27, 16, 4);  // I2C LCD 16x4

// Function to convert meters to feet and inches
void metersToFeetInches(float height_m, int &feet, int &inches) {
  float totalInches = height_m * 39.3701; // 1 meter = 39.3701 inches
  feet = int(totalInches / 12);
  inches = int(totalInches) % 12;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize HX711
  scale.begin(DT, SCK);
  scale.set_offset(offset);
  scale.set_scale(calibration_factor);
  scale.tare();

  // Initialize ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize pulse sensor pin
  pinMode(PULSE_PIN, INPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("BMI + Pulse Monitor");
  delay(2000);
  lcd.clear();
}

void loop() {
  // --- Read pulse sensor ---
  pulseValue = analogRead(PULSE_PIN);

  // Simple threshold to detect if finger is on sensor
  if (pulseValue > 550) { // adjust threshold according to your sensor
    fingerDetected = true;

    // Measure bpm (simple approx, can be improved with pulse library)
    static unsigned long lastBeatTime = 0;
    static int beatCount = 0;
    static unsigned long lastMeasure = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastBeatTime > 300) { // simple beat detection
      beatCount++;
      lastBeatTime = currentTime;
    }

    if (currentTime - lastMeasure >= 5000) { // calculate bpm every 5 sec
      bpm = beatCount * 12; // 60 sec / 5 sec = 12
      beatCount = 0;
      lastMeasure = currentTime;
    }

    // --- Display Pulse Rate ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pulse Sensor Active");
    lcd.setCursor(0, 1);
    lcd.print("Pulse Rate: ");
    lcd.print(bpm);
    lcd.print(" BPM");

    Serial.print("Pulse Value: "); Serial.print(pulseValue);
    Serial.print(" BPM: "); Serial.println(bpm);

    delay(200); // small delay to avoid flicker
    return; // skip BMI display while finger is detected
  } else {
    fingerDetected = false;
  }

  // --- BMI calculation (same as your existing code) ---
  // --- Weight from HX711 ---
  float weight_g = scale.get_units(20); // average 20 readings
  float weight_kg = weight_g / 1000.0;

  // --- Height from ultrasonic ---
  long duration;
  float height_m;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) duration = 1;

  float distance_m = duration * 0.000343 / 2.0;
  height_m = SENSOR_HEIGHT_M - distance_m;
  if (height_m < 0) height_m = 0;

  // Convert height to feet/inches
  int feet, inches;
  metersToFeetInches(height_m, feet, inches);

  // --- Calculate BMI ---
  float bmi = 0;
  if (height_m > 0) bmi = weight_kg / (height_m * height_m);

  // --- Determine BMI status ---
  String bmi_status;
  if (bmi < 18.5) bmi_status = "Under";
  else if (bmi < 25.0) bmi_status = "Normal";
  else if (bmi < 30.0) bmi_status = "Overweight";
  else bmi_status = "Obese";

  // --- Display BMI on LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(feet);
  lcd.print("'");
  lcd.print(inches);
  lcd.print("\" (");
  lcd.print(height_m, 2);
  lcd.print(" m)");

  lcd.setCursor(0, 1);
  lcd.print("Weight: ");
  lcd.print(weight_kg, 1);
  lcd.print(" kg");

  lcd.setCursor(0, 2);
  lcd.print("BMI: ");
  lcd.print(bmi, 1);

  lcd.setCursor(0, 3);
  lcd.print("Status: ");
  lcd.print(bmi_status);

  // --- Serial output ---
  Serial.print("Height: "); Serial.print(feet); Serial.print("'"); Serial.print(inches);
  Serial.print("\" ("); Serial.print(height_m, 2); Serial.print(" m), ");
  Serial.print("Weight: "); Serial.print(weight_kg,1); Serial.print(" kg, ");
  Serial.print("BMI: "); Serial.print(bmi,1); Serial.print(" Status: ");
  Serial.println(bmi_status);

  delay(500); // update every 0.5 sec
}

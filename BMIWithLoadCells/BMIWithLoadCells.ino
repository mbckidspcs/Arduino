  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include "HX711.h"

  // ------------------ HX711 ------------------
  #define DT 33
  #define SCK 32
  HX711 scale;

  // HX711 calibration (adjust as per your scale)
  float calibration_factor = 22.112848;
  long offset = 135632;

  // ------------------ Ultrasonic ------------------
  #define TRIG_PIN 4
  #define ECHO_PIN 5
  const float SENSOR_HEIGHT_M = 2.10; // sensor mounting height in meters

  // ------------------ LCD ------------------
  LiquidCrystal_I2C lcd(0x27, 16, 4);  // I2C LCD 16x4

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

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("BMI Calculator");
    delay(2000);
    lcd.clear();
  }

  // Function to convert meters to feet and inches
  void metersToFeetInches(float height_m, int &feet, int &inches) {
    float totalInches = height_m * 39.3701; // 1 meter = 39.3701 inches
    feet = int(totalInches / 12);
    inches = int(totalInches) % 12;
  }

  void loop() {
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

    float distance_m = duration * 0.000343 / 2.0; // speed of sound 343 m/s
    height_m = SENSOR_HEIGHT_M - distance_m;
    if (height_m < 0) height_m = 0;

    // Convert height to feet and inches
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

    // --- Display on 16x4 LCD ---
    lcd.clear();

    // Row 0: Height in feet/inches with meters in brackets
    lcd.setCursor(0, 0);
    lcd.print(feet);
    lcd.print("'");
    lcd.print(inches);
    lcd.print("\" (");
    lcd.print(height_m, 2);
    lcd.print(" m)");

    // Row 1: Weight
    lcd.setCursor(0, 1);
    lcd.print("Weight: ");
    lcd.print(weight_kg, 1);
    lcd.print(" kg");

    // Row 2: BMI
    lcd.setCursor(0, 2);
    lcd.print("BMI: ");
    lcd.print(bmi, 1);

    // Row 3: BMI status
    lcd.setCursor(0, 3);
    lcd.print("Status: ");
    lcd.print(bmi_status);

    // --- Serial output ---
    Serial.print("Height: "); Serial.print(feet); Serial.print("'"); Serial.print(inches);
    Serial.print("\" ("); Serial.print(height_m, 2); Serial.print(" m), ");
    Serial.print("Weight: "); Serial.print(weight_kg,1); Serial.print(" kg, ");
    Serial.print("BMI: "); Serial.print(bmi,1); Serial.print(" Status: ");
    Serial.println(bmi_status);

    delay(500); // update every 0.5 second
  }

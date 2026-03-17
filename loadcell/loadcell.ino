#include "HX711.h"

#define DT 21
#define SCK 22

HX711 scale;
float calibration_factor = -7050.0; // starting guess

void setup() {
  Serial.begin(115200);
  delay(2000); // allow HX711 to power up

  scale.begin(DT, SCK);
  scale.set_gain(128);

  // Wait until HX711 is ready
  Serial.println("Waiting for HX711...");
  if (!scale.wait_ready_timeout(5000)) {
    Serial.println("ERROR: HX711 not found. Check wiring/power!");
    while (1);
  }
  Serial.println("HX711 detected!");

  // Tare scale
  Serial.println("Remove all weight and press ENTER to tare...");
  while (!Serial.available()) delay(100);
  while (Serial.available()) Serial.read(); // clear buffer
  scale.tare();
  Serial.println("Tare done!");
  delay(500);

  // Auto-calibration
  Serial.println("Place known weight and type weight in grams (e.g., 28000 for 28kg):");
  while (!Serial.available()) delay(100);
  long known_weight = Serial.parseInt();
  while (Serial.available()) Serial.read(); // clear buffer

  // Wait HX711 ready before reading
  scale.wait_ready_timeout(2000);
  float raw_reading = scale.get_units(20); // average 20 readings
  calibration_factor = calibration_factor * raw_reading / known_weight;
  //scale.set_scale(calibration_factor);
   scale.set_offset(135632);
    scale.set_scale(22.112848);

  Serial.print("Calibration done! New factor: ");
  Serial.println(calibration_factor, 1);
  Serial.println("Starting continuous weight readings...");
}

void loop() {
  if (scale.is_ready()) {
    float weight_g = scale.get_units(20); // average 20 readings
    float weight_kg = weight_g / 1000.0;

    Serial.print("Weight: ");
    Serial.print(weight_kg, 3);
    Serial.println(" kg");
  } else {
    Serial.println("HX711 not ready...");
  }

  delay(500); // slow down Serial prints
}

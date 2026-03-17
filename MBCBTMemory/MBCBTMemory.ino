  #include "BluetoothSerial.h"
  #include <Preferences.h>

  Preferences prefs;
  BluetoothSerial SerialBT;

  #define RELAY_PIN   5
  #define SENSOR_PIN  34

  float sensitivity = 0.100;
  int adcMax = 4095;
  float vcc = 5.0;
  float currentLimit = 3.0;  // Default
  float vRef = 2.5;

  bool isRelayOn = true;

 // Turns the relay on and notifies the app
  void relayOn() {
    digitalWrite(RELAY_PIN, LOW); // Active LOW relay turns ON
    isRelayOn = true;
    Serial.println("Relay turned ON");
    SerialBT.println("R:1"); // Send Relay Status: 1 for ON
  }

  // Turns the relay off and notifies the app
  void relayOff() {
    digitalWrite(RELAY_PIN, HIGH); // Active LOW relay turns OFF
    isRelayOn = false;
    Serial.println("Relay turned OFF");
    SerialBT.println("R:0"); // Send Relay Status: 0 for OFF
  }

  // Measures the AC RMS current from the sensor
  float getCurrentRMS() {
    const int samples = 1000;
    long sumSq = 0;

    for (int i = 0; i < samples; i++) {
      int raw = analogRead(SENSOR_PIN);
      float voltage = (raw / (float)adcMax) * vcc;
      float offset = voltage - vRef;      // Remove DC bias
      float current = offset / sensitivity;
      sumSq += (current * current);
      delayMicroseconds(100); // Small delay for stability
    }

    float meanSq = (float)sumSq / samples;
    return sqrt(meanSq);
  }

  // Calibrates the sensor by finding the zero-current voltage (vRef)
  void calibrateSensor() {
    Serial.println("Calibrating sensor... Make sure no current is flowing.");
    long sum = 0;
    int samples = 2000;

    for (int i = 0; i < samples; i++) {
      sum += analogRead(SENSOR_PIN);
      delay(1);
    }

    float adc = (float)sum / samples;
    vRef = (adc / adcMax) * vcc;

    Serial.print("Calibration done. vRef = ");
    Serial.println(vRef, 3);
  }

  void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);

    SerialBT.begin("ESP32_Current_Monitor");
    Serial.println("Bluetooth device is ready to pair.");

    calibrateSensor();

    // --- Load current limit from flash ---
    prefs.begin("settings", false);
    currentLimit = prefs.getFloat("limit", 3.0);
    Serial.print("Loaded current limit: ");
    Serial.println(currentLimit, 2);

    relayOn();
  }

  void loop() {
    float current = getCurrentRMS();
    SerialBT.print("C:");
    SerialBT.println(current, 2);

    if (isRelayOn && current > currentLimit) {
      Serial.println("Overcurrent detected! Turning OFF relay.");
      relayOff();
    }

    if (SerialBT.available()) {
      String command = SerialBT.readStringUntil('\n');
      command.trim();
      
      if (command == "RELAY_ON") {
        relayOn();
      } else if (command == "RELAY_OFF") {
        relayOff();
      } else if (command == "STATUS_REQUEST") {
        SerialBT.print("C:");
        SerialBT.println(current, 2);
        SerialBT.print("R:");
        SerialBT.println(isRelayOn ? "1" : "0");
        SerialBT.print("L:");
        SerialBT.println(currentLimit, 2);   // Send current limit back
      } else if (command.startsWith("SET_LIMIT:")) {
        float newLimit = command.substring(10).toFloat();
        if (newLimit > 0 && newLimit < 20) {
          currentLimit = newLimit;
          prefs.putFloat("limit", currentLimit);
          Serial.print("New current limit saved: ");
          Serial.println(currentLimit, 2);
          SerialBT.print("LIMIT_SET:");
          SerialBT.println(currentLimit, 2);
        }
      }
    }

    delay(1000);
  }

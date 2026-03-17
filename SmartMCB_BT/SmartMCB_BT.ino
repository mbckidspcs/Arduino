  #include "BluetoothSerial.h"

  // Check if Bluetooth is supported
  #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to enable it
  #endif

  // Bluetooth Serial object
  BluetoothSerial SerialBT;

  // Pin Definitions
  #define RELAY_PIN   5
  #define SENSOR_PIN  34 // ADC1_CH6 on ESP32

  // Current Sensor Settings
  float sensitivity = 0.100; // 100 mV/A for 20A module
  int adcMax = 4095;         // ESP32 ADC is 12-bit
  float vcc = 5.0;           // Sensor supply voltage

  // Current Limit
  float currentLimit = 3.0;  // Amps

  // Calibration - will be set by calibrateSensor()
  float vRef = 2.5;

  // Global state for the relay
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

    // Start Bluetooth Serial with a unique name
    SerialBT.begin("ESP32_Current_Monitor");
    Serial.println("Bluetooth device is ready to pair.");

    calibrateSensor(); // Run with no load attached
    
    // Set initial relay state
    relayOn();
  }

  void loop() {
    // --- 1. Measure Current ---
    float current = getCurrentRMS();

    // Print to Serial for debugging
    Serial.print("Current RMS: ");
    Serial.print(current, 2);
    Serial.println(" A");

    // Send current data over Bluetooth in the format "C:value"
    SerialBT.print("C:");
    SerialBT.println(current, 2);


    // --- 2. Check for Overcurrent Condition ---
    if (isRelayOn && current > currentLimit) {
      Serial.println("Overcurrent detected! Turning OFF relay.");
      relayOff();
    }

    // --- 3. Handle Incoming Bluetooth Commands ---
    if (SerialBT.available()) {
      String command = SerialBT.readStringUntil('\n');
      command.trim(); // Remove any whitespace
      
      if (command == "RELAY_ON") {
        relayOn();
      } else if (command == "RELAY_OFF") {
        relayOff();
      } else if (command == "STATUS_REQUEST") {
        // App is requesting the current state, send it back
        SerialBT.print("C:");
        SerialBT.println(current, 2);
        SerialBT.print("R:");
        SerialBT.println(isRelayOn ? "1" : "0");
      }
    }

    delay(1000); // Wait for a second before the next loop
  }

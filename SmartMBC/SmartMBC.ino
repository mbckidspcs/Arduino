  #define RELAY_PIN  5
  #define SENSOR_PIN 34

  // ====== Current Sensor Settings ======
  float sensitivity = 0.100;  // 20A module = 100 mV/A
                            // 30A module = 66 mV/A
                            // 5A module  = 185 mV/A
  int adcMax = 4095;          // ESP32 ADC (12-bit)
  float vcc = 5.0;            // Sensor supply

  // ====== Current Limit ======
  float currentLimit = 3.0;   // Amps

  // ====== Calibration ======
  float vRef = 2.5;

  void relayOn() {
    digitalWrite(RELAY_PIN, LOW);   // Active LOW → ON
  }

  void relayOff() {
    digitalWrite(RELAY_PIN, HIGH);  // Active LOW → OFF
  }

  // ====== Get AC RMS Current ======
  float getCurrentRMS() {
    const int samples = 1000;
    long sumSq = 0;

    for (int i = 0; i < samples; i++) {
      int raw = analogRead(SENSOR_PIN);
      float voltage = (raw / (float)adcMax) * vcc;
      float offset = voltage - vRef;    // remove DC bias
      float current = offset / sensitivity;
      sumSq += (current * current);
    }

    float meanSq = (float)sumSq / samples;
    float rms = sqrt(meanSq);
    return rms;
  }

  void calibrateSensor() {
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
    relayOn();

    calibrateSensor();   // run with no load
  }

  void loop() {
    float current = getCurrentRMS();
    Serial.print("Current RMS: ");
    Serial.print(current, 2);
    Serial.println(" A");

    if (current > currentLimit) {
      Serial.println("Overcurrent! Turning OFF relay.");
      relayOff();
    }

    delay(1000);
  }

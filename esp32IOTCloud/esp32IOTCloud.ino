  #include "DHT.h"

  // Define sensor type and pin
  #define DHTPIN 4         // GPIO pin connected to the DHT sensor
  #define DHTTYPE DHT22    // Change to DHT11 if using DHT11

  DHT dht(DHTPIN, DHTTYPE);

  void setup() {
    Serial.begin(115200);
    Serial.println("DHT Sensor Test");
    dht.begin();
  }

  void loop() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();     // Celsius
    float f = dht.readTemperature(true); // Fahrenheit

    // Check if any reads failed
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(2000);
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %  Temperature: ");
    Serial.print(t);
    Serial.print(" °C  ");
    Serial.print(f);
    Serial.println(" °F");

    delay(2000); // Read every 2 seconds
  }

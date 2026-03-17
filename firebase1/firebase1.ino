    #include <WiFi.h>
    #include <FirebaseESP32.h>

    // Replace with your Wi-Fi credentials
    #define WIFI_SSID "Dialog 4G 713"
    #define WIFI_PASSWORD "3aF15bFF"

    #define API_KEY "AIzaSyD-Axka2DqrWc4Zkf1PdjvUtm7_KE0imCA"
    #define DATABASE_URL "https://my-iot-781d7-default-rtdb.firebaseio.com/"  // No "https://" and no trailing slash

    #define USER_EMAIL ""
    #define USER_PASSWORD ""

    #include <FirebaseESP32.h>

    // Create a FirebaseESP32 object instance
    FirebaseData fbData;
    FirebaseAuth auth;
    FirebaseConfig config;
    FirebaseESP32 client;

  void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(100);

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    client.begin(&config, &auth);
    client.reconnectWiFi(true);

    // Write and read using 'client'
      if (client.setFloat(fbData, "/test/value", 3.14)) {
      Serial.println("Value written");
      } else {
        Serial.println("Write failed: " + fbData.errorReason());
      }

      if (client.getFloat(fbData, "/test/value")) {
        Serial.println("Value read: " + String(fbData.floatData()));
      } else {
        Serial.println("Read failed: " + fbData.errorReason());
      }

  }

  void loop() {
    // Leave empty if nothing to do
  }


#include <WiFi.h>
#include <FirebaseESP32.h>


// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Replace with your Wi-Fi credentials
#define WIFI_SSID "F22"
#define WIFI_PASSWORD "12345678"

// Replace with your Firebase project credentials
#define API_KEY "AIzaSyD-Axka2DqrWc4Zkf1PdjvUtm7_KE0imCA"
#define DATABASE_URL "https://my-iot-781d7-default-rtdb.firebaseio.com/"
#define IR_SENSOR_PIN 5  // change this to the pin you used

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool ledState = false;
const int ledPin = 2;
int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up anonymously (required)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp successful");
  } else {
    Serial.printf("signUp Failed, reason: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  if (Firebase.getString(fbdo, "/led")) {
    String ledState = fbdo.stringData();
    digitalWrite(ledPin, ledState == "true" ? HIGH : LOW);
    //Serial.println(ledState ? "LED ON" : "LED OFF");
  } else {
    Serial.println("Path /led doesn't exist or error:");
    Serial.println(fbdo.errorReason());

    // Optional: Create it if missing
    Firebase.setString(fbdo, "/led", "false");
  }

   if (Firebase.getInt(fbdo, "/counter")) {
    int ledState = fbdo.intData();
    digitalWrite(ledPin, ledState == 50 ? HIGH : LOW);
    //Serial.println(ledState ? "LED ON" : "LED OFF");
  } 

  int state = digitalRead(IR_SENSOR_PIN);

  if (state == LOW) {
    Serial.println("Object Detected!");
    Firebase.setString(fbdo, "/IR", "image");
  } else {
    Serial.println("No Object.");
    Firebase.setString(fbdo, "/IR", " No Image");
  }



  delay(1000);  // Check every 2 seconds
}

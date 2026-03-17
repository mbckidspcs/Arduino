#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Dialog 4G 713";
const char* password = "123456789";

// MQTT
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topic = "kd/cart/control";

// Failsafe timeout (ms)
#define FAILSAFE_TIMEOUT 300

WiFiClient espClient;
PubSubClient client(espClient);

StaticJsonDocument<128> doc;

unsigned long lastMsg = 0;
bool failsafeActive = false;

// --------------------------------------------------

void stopCart() {
  if (failsafeActive) return;   // 🔒 latch

  failsafeActive = true;
  Serial.println("!!! FAILSAFE: CART STOPPED !!!");

  // 🚨 STOP YOUR CART HERE
  // digitalWrite(MOTOR_ENABLE, LOW);
  // ledcWrite(PWM_CH, 0);
}

// --------------------------------------------------

void callback(char*, byte* payload, unsigned int len) {
  DeserializationError error = deserializeJson(doc, payload, len);
  if (error) return;

  lastMsg = millis();

  // ---- NORMAL OPERATION ----
  // (No failsafe switch logic here)

  Serial.print("J1(");
  Serial.print(doc["jx"].as<int>());
  Serial.print(",");
  Serial.print(doc["jy"].as<int>());
  Serial.print(") ");

  Serial.print("J2(");
  Serial.print(doc["lx"].as<int>());
  Serial.print(",");
  Serial.print(doc["ly"].as<int>());
  Serial.print(") ");

  Serial.print("P(");
  Serial.print(doc["p1"].as<int>());
  Serial.print(",");
  Serial.print(doc["p2"].as<int>());
  Serial.print(") ");

  Serial.print("SW:");
  for (int i = 0; i < 8; i++) {
    Serial.print(doc["sw"][i].as<int>());
    Serial.print(" ");
  }
  Serial.println();

  // Communication OK → clear failsafe
  if (failsafeActive) {
    Serial.println(">>> COMMUNICATION RESTORED <<<");
    failsafeActive = false;
  }
}

// --------------------------------------------------

void reconnect() {
  while (!client.connected()) {
    client.connect("CART_RX");
    client.subscribe(topic);
    delay(300);
  }
}

// --------------------------------------------------

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

// --------------------------------------------------

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // ---- FAILSAFE ON TIMEOUT ----
  if (millis() - lastMsg > FAILSAFE_TIMEOUT) {
    stopCart();
  }
}

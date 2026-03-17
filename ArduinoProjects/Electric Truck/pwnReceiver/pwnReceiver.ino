#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi
const char* ssid = "mbc-edu";
const char* password = "mbcEdu@123";

// MQTT
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topic = "kd/rc/pwm";

WiFiClient espClient;
PubSubClient client(espClient);

// Motor pins
#define ENA 14
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33
#define ENB 32

int speedVal = 100;
int turnSpeed = 20;

// Minimal buffer for PWM string
char pwmValue[8] = "";
unsigned long lastMsg = 0;  // timestamp for failsafe

// --- Motor control ---
  void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
    Serial.println("forward");
  }

  void forwardRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, turnSpeed);
    analogWrite(ENB, speedVal);
     Serial.println("forwardRight");
  }

  void forwardLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, turnSpeed);
     Serial.println("forwardLeft");
  }

  void backwardLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, turnSpeed);
    analogWrite(ENB, speedVal);
     Serial.println("backwardLeft");
  }

  void backwardRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, turnSpeed);
     Serial.println("backwardRight");
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
     Serial.println("backward");
  }

  void left() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
     Serial.println("left");
  }

  void right() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
     Serial.println("right");
  }

  void stopCar() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
     Serial.println("stopCar");
  }
// -------- MQTT callback --------
void callback(char* topic, byte* payload, unsigned int length) {
  if(length >= sizeof(pwmValue)) length = sizeof(pwmValue)-1;
  memcpy(pwmValue, payload, length);
  pwmValue[length] = '\0';
  lastMsg = millis();
}

// -------- Setup --------
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopCar();

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) delay(200);
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

// -------- Loop --------
void loop() {
  // Non-blocking MQTT reconnect
  if(!client.connected()) {
    if(client.connect("ESP32_PWM_RX")) {
      client.subscribe(topic);
    }
  }
  client.loop();

  // Failsafe: stop car if no command for 200ms
  if(millis() - lastMsg > 500) {
    stopCar();
    return;
  }

  // Parse command prefix
  if(strlen(pwmValue) > 0) {
    char cmd = pwmValue[0];

    // Execute motor commands
    switch(cmd){
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'R': right(); break;
      case 'L': left(); break;
      case 'S': stopCar(); break;
      case 'H': forwardRight(); break;
      case 'G': forwardLeft(); break;
      case 'J': backwardRight(); break;
      case 'I': backwardLeft(); break;

      // Speed levels
      case '1': speedVal=80; break;
      case '2': speedVal=150; break;
      case '3': speedVal=200; break;
      case '4': speedVal=255; break;
    }
  }
}

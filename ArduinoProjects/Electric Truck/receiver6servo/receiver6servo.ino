#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define CHANNELS 6
const int pwmPins[CHANNELS] = {13,12,14,27,26,25};
Servo servos[CHANNELS];
uint16_t pwmValues[CHANNELS] = {1500,1500,1500,1500,1500,1500};
unsigned long lastMsg = 0;

const char* ssid = "mbc-edu";
const char* password = "mbcEdu@123";
const char* mqttServer="broker.hivemq.com";
const int mqttPort=1883;
const char* topic="kd/rc/pwm6";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length){
  if(length!=CHANNELS*2) return;
  memcpy(pwmValues,payload,length);
  lastMsg=millis();
}

void connectMQTT(){
  if(client.connected()) return;
  String cid="ESP32_RX_"+String((uint32_t)ESP.getEfuseMac(),HEX);
  if(client.connect(cid.c_str())) client.subscribe(topic);
}

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(50);

  client.setServer(mqttServer,mqttPort);
  client.setCallback(callback);

  for(int i=0;i<CHANNELS;i++) servos[i].attach(pwmPins[i]);
}

void loop() {
  connectMQTT();
  client.loop();

  // Check fail-safe timeout (e.g., 120 ms)
  bool failsafe = (millis() - lastMsg > 120);

  for (int i = 0; i < CHANNELS; i++) {
    uint16_t pwm = failsafe ? 1500 : pwmValues[i];      // use default if no signal
    servos[i].write(pwmToAngle(pwm));     // map to servo angle
  }

  // Debug print
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 50) {   // 20 Hz print
    lastPrint = millis();
    Serial.print("RX: ");
    for (int i = 0; i < CHANNELS; i++) Serial.print(failsafe ? 1500 : pwmValues[i]), Serial.print(" ");
    Serial.print(failsafe ? "| FAILSAFE" : "| OK");
    Serial.println();
  }
}

int pwmToAngle(int pwm){
  if(pwm > 1500) 
    return 98;

  if(pwm > 1450) 
    return 90;
  
  return 83;
}
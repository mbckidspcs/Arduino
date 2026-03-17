#include <WiFi.h>
#include <PubSubClient.h>

#define CHANNELS 6
const int pwmPins[CHANNELS] = {13,12,14,27,26,25};
const uint16_t FAILSAFE_US = 1500;
volatile uint16_t pwmValues[CHANNELS] = {FAILSAFE_US, FAILSAFE_US, FAILSAFE_US, FAILSAFE_US, FAILSAFE_US, FAILSAFE_US};
volatile uint32_t riseTime[CHANNELS] = {0};

const char* ssid = "mbc-edu";
const char* password = "mbcEdu@123";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topic = "kd/rc/pwm6";

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR pwmISR0(){ pwmValues[0] = digitalRead(pwmPins[0]) ? (riseTime[0]=micros(), pwmValues[0]) : micros()-riseTime[0]; }
void IRAM_ATTR pwmISR1(){ pwmValues[1] = digitalRead(pwmPins[1]) ? (riseTime[1]=micros(), pwmValues[1]) : micros()-riseTime[1]; }
void IRAM_ATTR pwmISR2(){ pwmValues[2] = digitalRead(pwmPins[2]) ? (riseTime[2]=micros(), pwmValues[2]) : micros()-riseTime[2]; }
void IRAM_ATTR pwmISR3(){ pwmValues[3] = digitalRead(pwmPins[3]) ? (riseTime[3]=micros(), pwmValues[3]) : micros()-riseTime[3]; }
void IRAM_ATTR pwmISR4(){ pwmValues[4] = digitalRead(pwmPins[4]) ? (riseTime[4]=micros(), pwmValues[4]) : micros()-riseTime[4]; }
void IRAM_ATTR pwmISR5(){ pwmValues[5] = digitalRead(pwmPins[5]) ? (riseTime[5]=micros(), pwmValues[5]) : micros()-riseTime[5]; }

void connectMQTT() {
  if(client.connected()) return;
  String cid = "ESP32_TX_"+String((uint32_t)ESP.getEfuseMac(), HEX);
  if(client.connect(cid.c_str())) client.loop();
}

void setup() {
  Serial.begin(115200);
  for(int i=0;i<CHANNELS;i++){
    pinMode(pwmPins[i], INPUT);
  }
  attachInterrupt(digitalPinToInterrupt(pwmPins[0]), pwmISR0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmPins[1]), pwmISR1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmPins[2]), pwmISR2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmPins[3]), pwmISR3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmPins[4]), pwmISR4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmPins[5]), pwmISR5, CHANGE);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(50);
  client.setServer(mqttServer,mqttPort);
}

void loop() {
  connectMQTT();
  client.loop();

  // publish every 20ms (~50Hz)
  static unsigned long lastPub = 0;
  if(millis()-lastPub>20){
    lastPub=millis();
    if(client.connected()){
      uint16_t temp[CHANNELS];
      for(int i=0;i<CHANNELS;i++) temp[i] = (pwmValues[i]<900||pwmValues[i]>2200)?FAILSAFE_US:pwmValues[i];
      client.publish(topic,(uint8_t*)temp,sizeof(temp),false);
    }
    Serial.print("TX: ");
    for(int i=0;i<CHANNELS;i++) Serial.print(pwmValues[i]), Serial.print(" ");
    Serial.println();
  }
}

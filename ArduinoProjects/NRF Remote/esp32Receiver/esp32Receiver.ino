  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>

  RF24 radio(4, 5); // CE, CSN
  const byte address[6] = "00001";

  struct DataPacket {
    int16_t x;
    int16_t y;
    bool btnA;
    bool btnB;
    bool btnC;
    bool btnD;
    bool btnE;
    bool btnF;
  };
  DataPacket data;

  // Motor pins
  #define ENA 14
  #define IN1 27
  #define IN2 26
  #define ENB 25
  #define IN3 33
  #define IN4 32

  // Dead zone (ignore small joystick noise)
  const int DEADZONE = 40;

  void setup() {
    Serial.begin(115200);
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_LOW);
    radio.startListening();

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    Serial.println("ESP32 Car Receiver Ready!");
  }

  void loop() {
    if (radio.available()) {
      radio.read(&data, sizeof(data));

      Serial.print("X: "); Serial.print(data.x);
      Serial.print(" | Y: "); Serial.print(data.y);
      Serial.print(" | A: "); Serial.print(data.btnA);
      Serial.print(" | B: "); Serial.print(data.btnB);
      Serial.print(" | C: "); Serial.print(data.btnC);
      Serial.print(" | D: "); Serial.print(data.btnD);
      Serial.print(" | E: "); Serial.print(data.btnE);
      Serial.print(" | F: "); Serial.println(data.btnF);

      // Map joystick range 0–675 (you measured it)
      int forwardBack = map(data.y, 0, 1024, -255, 255);
      int turn = map(data.x, 0, 1024, -150, 150);

      if (abs(forwardBack) < DEADZONE) forwardBack = 0;
      if (abs(turn) < DEADZONE) turn = 0;

      int leftSpeed = constrain(forwardBack - turn, -255, 255);
      int rightSpeed = constrain(forwardBack + turn, -255, 255);

      // Left motor
      if (leftSpeed > 0) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, leftSpeed);
      } else if (leftSpeed < 0) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        analogWrite(ENA, abs(leftSpeed));
      } else {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);
      }

      // Right motor
      if (rightSpeed > 0) {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        analogWrite(ENB, rightSpeed);
      } else if (rightSpeed < 0) {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        analogWrite(ENB, abs(rightSpeed));
      } else {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        analogWrite(ENB, 0);
      }
    }
  }

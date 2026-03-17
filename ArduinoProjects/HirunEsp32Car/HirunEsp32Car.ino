  #include "BluetoothSerial.h"

  BluetoothSerial SerialBT;

  // Motor driver pins
  #define ENA 14   // Left motor speed
  #define IN1 27
  #define IN2 26
  #define IN3 25
  #define IN4 33
  #define Buzz 32   // Right motor speed


  int speedVal = 255;   // default speed (0–255)
  int turnSpeed = 20;

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Hirun_BTCar");
    Serial.println("Bluetooth Car with Speed Control Ready!");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(Buzz, OUTPUT);

    stopCar();
  }

  // --- Motor control ---
  void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
   // analogWrite(ENB, speedVal);
  }

  void forwardRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, turnSpeed);
   // analogWrite(ENB, speedVal);
  }

  void forwardLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
  //  analogWrite(ENB, turnSpeed);
  }

  void backwardLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, turnSpeed);
   // analogWrite(ENB, speedVal);
  }

  void backwardRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
   // analogWrite(ENB, turnSpeed);
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
   // analogWrite(ENB, speedVal);
  }

  void left() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
  //  analogWrite(ENB, speedVal);
  }

  void right() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
   // analogWrite(ENB, speedVal);
  }

  void stopCar() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
   // analogWrite(ENB, 0);
  }


  void loop() {

    
    if (SerialBT.available()) {
      char command = SerialBT.read();
      Serial.print("Command: ");
      Serial.println(command);

      switch (command) {
        case 'F': forward(); break;
        case 'B': backward(); break;
        case 'R': right(); break;
        case 'L': left(); break;
        case 'S': stopCar(); break;
        case 'H': forwardRight(); break;
        case 'G': forwardLeft(); break;
        case 'J': backwardRight(); break;
        case 'I': backwardLeft(); break;

       case 'W': digitalWrite(Buzz, HIGH); break;
       case 'w': digitalWrite(Buzz, LOW); break;


        // Speed levels
        case '1': speedVal = 80;  break;  // Slow
        case '2': speedVal = 150; break;  // Medium
        case '3': speedVal = 200; break;  // Fast
        case '4': speedVal = 255; break;  // Full
      }
    }
  }

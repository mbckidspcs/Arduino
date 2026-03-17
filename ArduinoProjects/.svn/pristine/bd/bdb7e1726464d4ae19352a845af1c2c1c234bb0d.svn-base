  #include <Arduino.h>
  #include <ESP32Servo.h>
  #include "BluetoothSerial.h"
  // --- Motor Driver Pins ---
  
  BluetoothSerial SerialBT;
  #define IN1 26
  #define IN2 25
  #define IN3 33
  #define IN4 32

  #define ENA 22   // Left motor speed
  #define ENB 23   // Left motor speed


  int speedVal = 255;   // default speed (0–255)
  int turnSpeed = 20;

  // --- Ultrasonic Sensor Pins ---
  #define TRIG_PIN 14
  #define ECHO_PIN 12

  // --- Servo Pin ---
  #define SERVO_PIN 13

  Servo servoMotor;

  // --- Variables ---
  long duration;
  int distance;

  int speedMotor = 200; // 0-255 PWM speed
  int safeDistance = 25; // cm

  // --- Function Declarations ---
  // int getDistance();
  // void moveForward();
  // void moveBackward();
  // void turnLeft();
  // void turnRight();
  // void stopCar();

  void setup() {
   
    Serial.begin(115200);
    SerialBT.begin("Chenul's Car");
    Serial.println("Bluetooth Car with Speed Control Ready!");

    // Motor pins
   
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    // Ultrasonic pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Servo setup
    servoMotor.attach(SERVO_PIN);
    servoMotor.write(90); // center position

    Serial.println("ESP32 Ultrasonic Car Ready");
  }

  void loop() {

    if (SerialBT.hasClient()) {
      if (SerialBT.available()) {
        char command = SerialBT.read();
        Serial.print("Command: ");
        Serial.println(command);

        switch (command) {
          case 'F': moveForward(); break;
          case 'B': moveBackward(); break;
          case 'L': turnLeft(); break;
          case 'R': turnRight(); break;
          case 'S': stopCar(); break;
          case 'H': forwardRight(); break;
          case 'G': forwardLeft(); break;
          case 'J': backwardRight(); break;
          case 'I': backwardLeft(); break;




          // Speed levels
          case '1': speedVal = 80;  break;  // Slow
          case '2': speedVal = 150; break;  // Medium
          case '3': speedVal = 200; break;  // Fast
          case '4': speedVal = 255; break;  // Full
        }
      
      }
    
    }else{

      int distCenter = getDistance();
      Serial.print("Center Distance: ");
      Serial.println(distCenter);

      if (distCenter > safeDistance) {
        moveForward();
      } else {
        stopCar();
        delay(300);
        
        // Look left
        servoMotor.write(150);
        delay(400);
        int distLeft = getDistance();
        Serial.print("Left Distance: ");Serial.println(distLeft);
      

        // Look right
        servoMotor.write(30);
        delay(400);
        int distRight = getDistance();
        Serial.print("Right Distance: ");Serial.println(distRight);

        // Back to center
        servoMotor.write(90);
        delay(200);

        if (distLeft > distRight && distLeft > safeDistance) {
          turnLeft();
          Serial.print("turnLeft");
          delay(300);
        } else if (distRight > safeDistance) {
          turnRight();
          Serial.print("turnRight");
          delay(300);
        } else {
          moveBackward();
          Serial.print("MoveBack");
          delay(300);
        }
      }
    }

  }

  // --- Distance Function ---
  int getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);
    int distance = duration * 0.034 / 2;
    return distance;
  }

  // --- Motor Functions ---
  void moveForward() {
    Serial.println("FWD");
    

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  void moveBackward() {
   

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  void turnRight() {
    

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  void turnLeft() {
    

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  void stopCar() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    
  }

  void forwardLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, turnSpeed);
    analogWrite(ENB, speedVal);
  }

  void forwardRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, turnSpeed);
  }

  void backwardRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, turnSpeed);
    analogWrite(ENB, speedVal);
  }

  void backwardLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, turnSpeed);
  }

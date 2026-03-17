  
  #include <Arduino.h>
  #include <Stepper.h>
  #include <ESP32Servo.h>   // ESP32-compatible servo library

  // ================== L298N Motor Driver Pins ==================
  #define ENA 14   // Enable A (PWM Left Motor)
  #define IN1 27
  #define IN2 26
  #define IN3 25
  #define IN4 33
  #define ENB 32   // Enable B (PWM Right Motor)

  // ================== Stepper Motor Pins ==================
  #define STEPPER_IN1 19
  #define STEPPER_IN2 18
  #define STEPPER_IN3 5
  #define STEPPER_IN4 17

  // ================== Servo Pins ==================
  #define SERVO1_PIN 21
  #define SERVO2_PIN 22
  #define SERVO3_PIN 23

  // ================== Relay Pins ==================
  #define RELAY1_PIN 4
  #define RELAY2_PIN 16   // second relay

  // ================== Globals ==================
  int motorSpeed = 200; // 0–255 PWM for DC motors
  int stepsPerRevolution = 500; // 28BYJ-48 Stepper motor typical value

  Stepper stepperMotor(stepsPerRevolution, STEPPER_IN1, STEPPER_IN3, STEPPER_IN2, STEPPER_IN4);
  Servo servo1, servo2, servo3;   // ESP32Servo objects

  // ================== Setup ==================
  void setup() {
    Serial.begin(115200);

    // Motor pins
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    // Stepper motor
    stepperMotor.setSpeed(15); // RPM

    // Servos
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    servo3.attach(SERVO3_PIN);

    // Relays
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
  }

  // ================== DC Motor Functions ==================
  void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  void left() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  void right() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  void stopMotors() {
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }

  // ================== Stepper Motor Function ==================

  void stepperMotorMove(int speed) {
    
    stepperMotor.setSpeed(speed); // RPM
    stepperMotor.step(stepsPerRevolution);
  }

  // ================== Servo Functions ==================
  // 50-160 degree
  void ServoRotory(int angle) {
    servo1.write(angle);
  }

  // 90-65 degree
  void ServoDigging(int angle) {
    servo2.write(angle);
  }

  // 180-110 degree
  void ServoCovering(int angle) {
    servo3.write(angle);
  }

  // ================== Relay Functions ==================
  void rotoryMotor(bool state) {
    digitalWrite(RELAY1_PIN, state ? HIGH : LOW);
  }

  void waterPump(bool state) {
    digitalWrite(RELAY2_PIN, state ? HIGH : LOW);
  }

  // ================== Loop ==================
  void loop() {
    Demo sequence
    forward();
    delay(2000);
    stopMotors();
    delay(500);

    backward();
    delay(2000);
    stopMotors();
    delay(500);

    left();
    delay(1500);
    stopMotors();
    delay(500);

    right();
    delay(1500);
    stopMotors();
    delay(500);

    stepperMotorMove(stepsPerRevolution); // 1 full rotation
    delay(2000);

    ServoRotory(50);
    delay(2000);
    ServoDigging(180);
    delay(2000);
    ServoCovering(180);
    delay(2000);
    

    rotoryMotor(true);
    waterPump(false);
    delay(2000);

    rotoryMotor(false);
    waterPump(true);
    delay(2000);
    
  }

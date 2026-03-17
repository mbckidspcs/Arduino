#include <AFMotor.h>

// Motor setup for Adafruit Motor Shield v1
AF_DCMotor motor1(1);  // Left Motor (M1)
AF_DCMotor motor2(2);  // Right Motor (M2)

char command = 'S';     // Default stop
int motorSpeed = 250;   // Fixed speed (0–255)

void setup() {
  Serial.begin(9600);   // Bluetooth connected to pins 0 (RX) and 1 (TX)
  Serial.println("Adafruit Bluetooth Car Ready!");

  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  stopCar();
}

void loop() {
  // Check for new command
  if (Serial.available() > 0) {
    command = Serial.read();
    Serial.print("Command: ");
    Serial.println(command);
  }

  // Execute last received command continuously
  switch (command) {
    case 'F': forward(); break;
    case 'B': backward(); break;
    case 'H': forwardRight(); break;
    case 'G': forwardLeft(); break;
    case 'J': backwardRight(); break;
    case 'I': backwardLeft(); break;
    case 'L': left(); break;
    case 'R': right(); break;
    case 'S': stopCar(); break;
    //default: stopCar(); break;
  }
}

// ===== Motor control functions =====

void forward() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
}

void forwardLeft() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(50);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  Serial.print("FWD L: ");
}

void forwardRight() {
  motor1.setSpeed(50);
  motor2.setSpeed(motorSpeed);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  Serial.print("FWD R: ");
}

void backwardLeft() {
  motor1.setSpeed(motorSpeed-200);
  motor2.setSpeed(motorSpeed);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  
}

void backwardRight() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed-200);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
}

void backward() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
}

void left() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
}

void right() {
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor1.run(BACKWARD);
  motor2.run(FORWARD);
}

void stopCar() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
}

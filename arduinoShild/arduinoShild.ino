  #include <AFMotor.h>

  // --- Define motors using Adafruit Motor Shield channels ---
  AF_DCMotor motor1(1); // M1
  AF_DCMotor motor2(2); // M2
  AF_DCMotor motor3(3); // M3
  AF_DCMotor motor4(4); // M4

  int speed = 200;

  char command;

  void setup() {
    Serial.begin(9600);   // HC-05 default baud rate
    Serial.println("Bluetooth Car Ready");    
    setSpeed();
  }

  void setSpeed(){
    // Set initial speed
    motor1.setSpeed(speed);
    motor2.setSpeed(speed);
    motor3.setSpeed(speed);
    motor4.setSpeed(speed);
  }

  // --- Movement functions ---
  void forward() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  }

  void backward() {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }

  void left() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }

    void Forwardleft() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);

    motor3.setSpeed(speed - 100);
    motor4.setSpeed(speed - 100);

    motor3.run(FORWARD);
    motor4.run(FORWARD);

    motor3.setSpeed(speed);
    motor4.setSpeed(speed);

  }

   void Backwardleft() {

    motor1.run(BACKWARD);
    motor2.run(BACKWARD);

    motor3.setSpeed(speed - 100);
    motor4.setSpeed(speed - 100);

    motor3.run(BACKWARD);
    motor4.run(BACKWARD);

    motor3.setSpeed(speed);
    motor4.setSpeed(speed);

  }

  void BackwardRight() {

    motor3.run(BACKWARD);
    motor4.run(BACKWARD);

    motor1.setSpeed(speed - 100);
    motor2.setSpeed(speed - 100);

    motor1.run(BACKWARD);
    motor2.run(BACKWARD);

    motor1.setSpeed(speed);
    motor2.setSpeed(speed);

  }

  void right() {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  }

   void Forwardright() {

    motor1.setSpeed(speed - 100);
    motor2.setSpeed(speed - 100);

    motor1.run(FORWARD);
    motor2.run(FORWARD);

    motor1.setSpeed(speed);
    motor2.setSpeed(speed);

    motor3.run(FORWARD);
    motor4.run(FORWARD);
  }

  void stopMotors() {
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    motor3.run(RELEASE);
    motor4.run(RELEASE);
  }

  // --- Main loop ---
  void loop() {
    if (Serial.available()) {
      command = Serial.read();
      Serial.print("Received: ");
      Serial.println(command);

      switch (command) {
        case 'F': forward(); break;
        case 'B': backward(); break;
        case 'L': left(); break;
        case 'R': right(); break;
        case 'H': Forwardright(); break;
        case 'G': Forwardleft(); break;
        case 'J': BackwardRight(); break;
        case 'I': Backwardleft(); break;

        case 'S': stopMotors(); break;

        case '1' : speed = 100; setSpeed(); break;
        case '2' : speed = 150; setSpeed(); break;
        case '3' : speed = 200; setSpeed(); break;
        case '4' : speed = 255; setSpeed(); break;
      }
    }
  }

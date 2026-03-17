  #include "BluetoothSerial.h"

  BluetoothSerial SerialBT;

  // -------- Motor driver pins --------
  #define ENA 14   // Left motor speed
  #define IN1 27
  #define IN2 26
  #define IN3 25
  #define IN4 33
  #define ENB 32   // Right motor speed

  // -------- Ultrasonic pins --------
  #define TRIG_PIN 13   // OUTPUT
  #define ECHO_PIN 12   // INPUT

  int speedVal = 255;   // default speed (0–255)
  int turnSpeed = 120;
 int dist = 100; 

  long duration;
  int distance;

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Manidu's Car");
    Serial.println("Bluetooth Car with Ultrasonic Ready!");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    stopCar();
  }

  // -------- Ultrasonic distance --------
  int getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
    distance = duration * 0.034 / 2;

    if (distance == 0) distance = 100; // no echo safety
    return distance;
  }

  // -------- Motor control --------
  void forward() {

    //   if (dist < 20) {
    //   Serial.println("Obstacle detected! Car stopped.");
    //   stopCar();
    //   delay(100);
    //   return;   // ignore Bluetooth commands
    // }

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void left() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void right() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void forwardLeft() {
    forward();
    analogWrite(ENB, turnSpeed);
  }

  void forwardRight() {
    forward();
    analogWrite(ENA, turnSpeed);
  }

  void backwardLeft() {
    backward();
    analogWrite(ENA, turnSpeed);
  }

  void backwardRight() {
    backward();
    analogWrite(ENB, turnSpeed);
  }

  void stopCar() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }

  // -------- Main Loop --------
  void loop() {

     dist = getDistance();
    Serial.print("Distance: ");
    Serial.println(dist);

   

    if (SerialBT.available()) {
      char command = SerialBT.read();
      Serial.print("Command: ");
      Serial.println(command);

       // 🚨 Auto stop if obstacle detected
  

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

        // Speed control
        case '1': speedVal = 80;  break;
        case '2': speedVal = 150; break;
        case '3': speedVal = 200; break;
        case '4': speedVal = 255; break;
      }
    }
  }

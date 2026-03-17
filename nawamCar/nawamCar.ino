  #include <BluetoothSerial.h>
  #include <ESP32Servo.h>

  BluetoothSerial SerialBT;
  Servo steeringServo;

  // Motor pins
  #define ENA 14
  #define IN1 27
  #define IN2 26

  // Servo pin
  #define SERVO_PIN 25

  // Speed variable
  int motorSpeed = 250;  // 0–255
  int servoCenter = 15;  // adjust if not centered

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Nawam_Car"); // Bluetooth name

    // Motor setup
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // Servo setup
    steeringServo.attach(SERVO_PIN);
    steeringServo.write(servoCenter);

    Serial.println("Bluetooth Car Ready!");
  }

  void loop() {
    if (SerialBT.available()) {
      char command = SerialBT.read();
      Serial.println(command);
      
      switch (command) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': turnLeft(); break;
      case 'R': turnRight(); break;
      case 'S': stopMotor(); break;
      case 'H': turnRight(); forward(); break;
      case 'G': turnLeft(); forward(); break;
      case 'I': turnLeft(); backward();  break;
      case 'J': turnRight(); backward(); break;
       
      }
    }
  }

  // Motor control
  void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, motorSpeed);
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, motorSpeed);
  }

  void stopMotor() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    steeringServo.write(servoCenter);
  }

  // Servo control
  void turnLeft() {
    steeringServo.write(servoCenter - 30); // adjust angle as needed
  }

  void turnRight() {
    steeringServo.write(servoCenter + 30); // adjust angle as needed
  }

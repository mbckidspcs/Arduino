  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>

  BluetoothSerial SerialBT;

  // Motor driver pins
  #define ENA 10   // Left motor speed
  #define IN1 9
  #define IN2 46
  #define IN3 3
  #define IN4 8
  #define ENB 18   // Right motor speed

// --- Servo pin ---
#define SERVO_PIN 13   // You can change to any PWM-capable pin

int speedVal = 255;   // default speed

long duration;
float distanceCm;

bool carMoving = false;  // track movement state

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Yashila_Car");
  Serial.println("Bluetooth Car + Servo + Ultrasonic Ready!");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(GRS, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  servo.attach(SERVO_PIN);  // attach servo
  servo.write(90);          // center position at startup

  stopCar();
}

// --- Ultrasonic function ---
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms timeout
  if (duration == 0) return 999; // No reading = assume far
  return duration * 0.034 / 2;   // in cm
}

// --- Motor control ---
void forward() {
  distanceCm = getDistance();
  if (distanceCm > 20) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
    carMoving = true;
  } else {
    stopCar();
    Serial.println("Obstacle detected! Stopped.");
  }
}

void forwardLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedVal / 4);
  analogWrite(ENB, speedVal);
  carMoving = true;
}

void forwardRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal / 4);
  carMoving = true;
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal);
  carMoving = true;
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  carMoving = false;
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal);
  carMoving = true;
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal);
  carMoving = true;
}

void GrassCutterON() {
  digitalWrite(GRS, HIGH);
  analogWrite(ENG, fanSpeed);
}

void GrassCutterOFF() {
  digitalWrite(GRS, LOW);
  analogWrite(ENG, 0);
}

// --- Servo sweep when car moves ---
void sweepServo() {
  static int pos = 0;
  static int dir = 1; // 1 = forward, -1 = backward
  pos += dir;
  servo.write(pos);

  if (pos >= 180) dir = -1;
  if (pos <= 0) dir = 1;
 // delay(10); // adjust sweep speed
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();
    Serial.print("Command: ");
    Serial.println(command);

    switch (command) {
      case 'F': forward(); break;
      case 'B': backward(); break;
      case 'L': left(); break;
      case 'R': right(); break;
      case 'S': stopCar(); break;
      case 'H': forwardRight(); break;
      case 'G': forwardLeft(); break;

      case 'V': fanSpeed = 100;  GrassCutterON(); break;
      case 'v': GrassCutterOFF(); break;
      case 'U': fanSpeed = 130;  GrassCutterON(); break;
      case 'u': GrassCutterOFF(); break;
      case 'W': fanSpeed = 150;  GrassCutterON(); break;
      case 'w': GrassCutterOFF(); break;
      case 'X': fanSpeed = 200; GrassCutterON(); break;
      case 'x': GrassCutterOFF(); break;

      // Speed levels
      case '1': speedVal = 80; break;
      case '2': speedVal = 150; break;
      case '3': speedVal = 200; break;
      case '4': speedVal = 255; break;
    }
  }

  // If the car is moving, sweep the servo continuously
  if (carMoving) {
    sweepServo();
  }
}



  #include "BluetoothSerial.h"
  #include <ESP32Servo.h>

  BluetoothSerial SerialBT;
  Servo servo;

  // --- Motor driver pins ---
  #define ENA 14
  #define IN1 27
  #define IN2 26
  #define IN3 25
  #define IN4 33
  #define ENB 32

  #define GRS 13
  #define ENG 12

  // --- Ultrasonic sensor pins ---
  #define TRIG_PIN 5
  #define ECHO_PIN 18

  // --- Servo pin ---
  #define SERVO_PIN 13

  int speedVal = 255;
  int fanSpeed = 200;
  long duration;
  float distanceCm;
  bool carMoving = false;

  // --- FreeRTOS Task handle ---
  TaskHandle_t ServoTaskHandle;

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Dulin_Smart");
    Serial.println("Bluetooth Car + Servo Thread Ready!");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(GRS, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    servo.attach(SERVO_PIN);
    servo.write(90);

    stopCar();

    // ✅ Create a new task on Core 0 for servo movement
    xTaskCreatePinnedToCore(
      servoTask,        // Function
      "Servo Task",     // Task name
      2048,             // Stack size
      NULL,             // Parameters
      1,                // Priority
      &ServoTaskHandle, // Task handle
      0                 // Core 0
    );
  }

  // --- Servo sweeping task ---
  void servoTask(void *pvParameters) {
    int pos = 0;
    int dir = 1;

    while (true) {
      if (carMoving) {
        pos += dir;
        servo.write(pos);
        if (pos >= 180) dir = -1;
        if (pos <= 0) dir = 1;
        vTaskDelay(10 / portTICK_PERIOD_MS);
      } else {
        // Keep servo centered when car stops
        servo.write(90);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    }
  }

  // --- Ultrasonic distance ---
  float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 20000);
    if (duration == 0) return 999;
    return duration * 0.034 / 2;
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
      Serial.println("Obstacle detected!");
    }
  }

  void forwardLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speedVal / 4);
    analogWrite(ENA, speedVal);
    carMoving = true;
  }

  void forwardRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speedVal);
    analogWrite(ENA, speedVal / 4);
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
    digitalWrite(IN2, LOW);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN4, HIGH);
    digitalWrite(IN3, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
    carMoving = true;
  }

  void right() {
    digitalWrite(IN2, HIGH);
    digitalWrite(IN1, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(IN3, HIGH);
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
  }

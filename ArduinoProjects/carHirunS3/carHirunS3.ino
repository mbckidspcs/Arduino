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



  int speedVal = 255;   // default speed (0–255)


  long duration;
  float distanceCm;

  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Hirun_BT_Car");
    Serial.println("Bluetooth Car with Speed Control Ready!");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    stopCar();
  }

  // --- Motor control ---
  void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void forwardLeft() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal / 4);
    analogWrite(ENB, speedVal);
  }

  void forwardRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal / 4);
  }

  void backwardRight() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal / 4);
    analogWrite(ENB, speedVal);
  }

  void backwardLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal / 4);
  }

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void right() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void left() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
  }

  void stopCar() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
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
        case 'J': backwardRight(); break;
        case 'I': backwardLeft(); break;



        // Speed levels
        case '1': speedVal = 80;  break;  // Slow
        case '2': speedVal = 150; break;  // Medium
        case '3': speedVal = 200; break;  // Fast
        case '4': speedVal = 255; break;  // Full
      }
    }
  }

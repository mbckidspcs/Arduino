  #include "BluetoothSerial.h"
  #include <SPI.h>
  #include <MFRC522.h>

  BluetoothSerial SerialBT;

  // --- Motor driver pins ---
  #define ENA 14   // Left motor speed
  #define IN1 27
  #define IN2 26
  #define IN3 25
  #define IN4 33
  #define ENB 32   // Right motor speed

  #define led 13
  #define ENG 12   // Grass cutter motor PWM pin

  // --- RFID Pins ---
  #define SS_PIN 5
  #define RST_PIN 22
  MFRC522 rfid(SS_PIN, RST_PIN);

  // --- Variables ---
  String allowedUID = "960F8A57";   // authorized card UID (uppercase, HEX string)
  bool authorized = false;

  int speedVal = 255;   // default speed
  int fanSpeed = 200;

  float distanceCm;

  // -------- SETUP --------
  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Risara_Samart");
    Serial.println("Bluetooth Car with RFID Access Control");

    // Motors
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(led, OUTPUT);

    stopCar();

    // RFID init
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("Scan your RFID card...");
  }

  // -------- LOOP --------
  void loop() {
    // --- RFID Check ---
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String uidString = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) uidString += "0";
        uidString += String(rfid.uid.uidByte[i], HEX);
      }
      uidString.toUpperCase();

      Serial.print("Card UID: ");
      Serial.println(uidString);

      if (uidString == allowedUID && !authorized) {
        authorized = true;
        Serial.println("✅ Access Granted! Car is unlocked.");
        digitalWrite(13, HIGH);
      } else {
        authorized = false;
        stopCar();
         digitalWrite(13, LOW);
        Serial.println("❌ Access Denied!");
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }

    // --- Bluetooth Command ---
    if (SerialBT.available()) {
      char command = SerialBT.read();
      Serial.print("Command: ");
      Serial.println(command);

      // Only allow control if authorized
      if (authorized) {
        switch (command) {
          case 'F': forward(); break;
          case 'B': backward(); break;
          case 'R': left(); break;
          case 'L': right(); break;
          case 'S': stopCar(); break;
          case 'H': forwardLeft(); break;
          case 'G': forwardRight(); break;         
       

          // Speed levels
          case '1': speedVal = 80; break;
          case '2': speedVal = 150; break;
          case '3': speedVal = 200; break;
          case '4': speedVal = 255; break;
        }
      } else {
        Serial.println("Car locked! Scan authorized RFID card first.");
        stopCar();
      }
    }
  }

  // -------- MOTOR FUNCTIONS --------
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

  void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
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

  void left() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
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



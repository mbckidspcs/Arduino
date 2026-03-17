  #include <SPI.h>
  #include <MFRC522.h>
  #include <Servo.h>

  #define SS_PIN 10
  #define RST_PIN 9

  MFRC522 mfrc522(SS_PIN, RST_PIN);

  Servo servo1;
  Servo servo2;

  // Authorized RFID UID (replace with your card UID)
  byte authorizedUID[4] = {0x2A , 0xF1 , 0xD6 , 0x05};
  byte authorizedUID2[4] = {0x77 , 0xA1 , 0x8A , 0x04};

  void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();

    servo1.attach(5);
    servo2.attach(6);

    servo1.write(0);   // Locked position
    servo2.write(0);

    Serial.println("Place RFID card...");
  }

  void loop() {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (checkUID()) {
      Serial.println("Access Granted");
      openServos();
    } else {
      Serial.println("Access Denied");
    }

    mfrc522.PICC_HaltA();
  }

  bool checkUID() {
    for (byte i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != authorizedUID2[i]) {
        return false;
      }
    }
    return true;
  }

  void openServos() {
    servo1.write(90);
    servo2.write(90);
    delay(3000);        // Open for 3 seconds
    servo1.write(0);
    servo2.write(0);
  }

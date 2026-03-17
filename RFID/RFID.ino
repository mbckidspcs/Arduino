

  #include <SPI.h>
  #include <MFRC522.h>
  #include <Servo.h>

  #define RST_PIN 9     // RST pin
  #define SS_PIN 10     // SDA pin
  #define SERVO_PIN 3   // Servo pin

  MFRC522 mfrc522(SS_PIN, RST_PIN);  
  Servo myServo;

  // Change this UID to your card/tag UID
    byte authorizedUIDs[][4] = {
      {0x45, 0x1E, 0xD6, 0x05},  // First card
      //{0x4B, 0x15, 0x9E, 0x04}   // Second card
    };
    
  const int totalAuthorized = sizeof(authorizedUIDs) / 4;  // number of cards

  void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    myServo.attach(SERVO_PIN);
    myServo.write(0);  // Locked position
    Serial.println("Place your card...");
  }

  void loop() {
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    // Print UID
    Serial.print("UID tag: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Check if authorized
    if (isAuthorized(mfrc522.uid.uidByte)) {
      Serial.println("Access Granted ✅");
      myServo.write(90);  // Unlock
      delay(2000);
      myServo.write(0);   // Lock again
    } else {
      Serial.println("Access Denied ❌");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  bool isAuthorized(byte *uid) {
    for (int card = 0; card < totalAuthorized; card++) {
      bool match = true;
      for (byte i = 0; i < 4; i++) {
        if (uid[i] != authorizedUIDs[card][i]) {
          match = false;
          break;
        }
      }
      if (match) return true;  // found a match
    }
    return false;  // no match found
  }

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED Config ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Initialize display (address will be set after detecting)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= RFID Config ====================
#define SS_PIN 5    // SDA
#define RST_PIN 4   // Reset
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ================= Book Setup =====================
struct Book {
  const char* name;
  byte uid[4];
  bool scanned;
};

Book books[] = {
  {"Science", {0x45, 0x1E, 0xD6, 0x05}, false},
  {"Maths",   {0xA3, 0x4B, 0x22, 0x19}, false},
  {"English", {0x91, 0x72, 0x3A, 0xE7}, false}
};
int totalBooks = sizeof(books) / sizeof(books[0]);

// ================= Helper Functions ===============
bool compareUID(byte *uid1, byte *uid2) {
  for (byte i = 0; i < 4; i++) if (uid1[i] != uid2[i]) return false;
  return true;
}

// Update OLED display
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  bool allScanned = true;
  display.println("Missing Books:");

  for (int i = 0; i < totalBooks; i++) {
    if (!books[i].scanned) {
      display.print("- ");
      display.println(books[i].name);
      allScanned = false;
    }
  }

  if (allScanned) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println("All Books");
    display.setCursor(0, 40);
    display.println("Inserted!");
  }

  display.display();
}

// ================== Setup =========================
void setup() {
  Serial.begin(115200);

  // Initialize I2C with custom pins
  Wire.begin(21, 22); // SDA, SCL

  // Detect OLED address
  byte oledAddress = 0x3C; // default
  Wire.beginTransmission(oledAddress);
  if (Wire.endTransmission() != 0) {
    Serial.println("OLED not found at 0x3C, trying 0x3D...");
    oledAddress = 0x3D;
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, oledAddress)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart School Bag");
  display.setCursor(0, 20);
  display.println("Scan your books...");
  display.display();
  delay(2000);

  // Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Scan books...");
  updateDisplay();
}

// ================== Loop ==========================
void loop() {
  // Look for new card
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Get UID
  byte *uid = mfrc522.uid.uidByte;
  Serial.print("Scanned UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(uid[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Match UID with book list
  for (int i = 0; i < totalBooks; i++) {
    if (!books[i].scanned && compareUID(uid, books[i].uid)) {
      books[i].scanned = true;
      Serial.print("Book Scanned: ");
      Serial.println(books[i].name);
    }
  }

  updateDisplay();
  mfrc522.PICC_HaltA();
}

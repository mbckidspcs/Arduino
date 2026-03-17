  #include <SPI.h>
  #include <MFRC522.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

  #define SS_PIN 21
  #define RST_PIN 22
  MFRC522 mfrc522(SS_PIN, RST_PIN);

  struct Book {
    const char* name;
    byte uid[4];
    bool scanned;
  };

  // 10 subjects
  Book books[] = {
    {"Buddhism",  {0x43,0x28,0xD4,0x95}, false},
    {"Sinhala",   {0xF7,0x6E,0x24,0x85}, false},
    {"English",   {0xF3,0x4F,0x9B,0xA9}, false},
    {"Maths",     {0x64,0x29,0x4D,0xDF}, false},
    {"Science",   {0x07,0xC7,0xBC,0x86}, false},
    {"History",   {0xB4,0x94,0x65,0xDF}, false},
    {"Geography", {0x33,0xF7,0x1B,0x96}, false},
    {"Civic",     {0x63,0xBE,0x64,0xA9}, false},
    {"Music",     {0x03,0x09,0x2C,0x95}, false},
    {"Tamil",     {0x43,0x67,0x7A,0x0E}, false}
  };

  int totalBooks = sizeof(books)/sizeof(books[0]);

  // Timetable (shuffled manually for each day)
  const char* timetable[5][8] = {
    {"Buddhism","Sinhala","English","Maths","Science","History","Geography","Civic"},
    {"Maths","Science","English","Tamil","Buddhism","History","Music","Civic"},
    {"Sinhala","English","Maths","Science","Civic","Music","History","Tamil"},
    {"English","Tamil","Buddhism","Maths","Science","History","Geography","Music"},
    {"Maths","Sinhala","English","Civic","Science","Buddhism","Music","Tamil"}
  };
  const char* dayNames[5] = {"Monday","Tuesday","Wednesday","Thursday","Friday"};
  int currentDayIndex = 0;

  // Extra books array
  const char* extraBooks[10];
  int extraCount = 0;

  // Scrolling variables
  int scrollOffset = 0;
  unsigned long lastScroll = 0;
  const int scrollSpeed = 300; // ms per scroll step

  bool compareUID(byte *uid1, byte *uid2) {
    for (byte i=0; i<4; i++) if (uid1[i] != uid2[i]) return false;
    return true;
  }

  bool isInTodaysTimetable(const char* name) {
    for (int i=0; i<8; i++) {
      if (strcmp(name, timetable[currentDayIndex][i]) == 0) return true;
    }
    return false;
  }

  bool isAlreadyExtra(const char* name) {
    for (int i=0; i<extraCount; i++) {
      if (strcmp(extraBooks[i], name) == 0) return true;
    }
    return false;
  }

  void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);

    const char** daySubjects = timetable[currentDayIndex];

    // Header
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.println(String("Time Table : ") + dayNames[currentDayIndex]);

    // Show 8 subjects in 2 columns → (0–3 left, 4–7 right)
    for (int row = 0; row < 4; row++) {
      for (int col = 0; col < 2; col++) {
        int idx = row + col * 4;
        bool scanned = false;

        for (int j = 0; j < totalBooks; j++) {
          if (strcmp(books[j].name, daySubjects[idx]) == 0 && books[j].scanned) {
            scanned = true;
            break;
          }
        }

        int x = (col == 0) ? 0 : 64;
        int y = 10 + row * 8; // reduced row spacing to fit extra line

        if (scanned) {
          display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // inverted
        } else {
          display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(x, y);
        display.println(daySubjects[idx]);
      }
    }

    display.setTextColor(SSD1306_WHITE);

    // Show extra books at the bottom (always)
    if (extraCount > 0) {
      String extraText = "Extra: ";
      for (int i = 0; i < extraCount; i++) {
        extraText += extraBooks[i];
        if (i < extraCount - 1) extraText += ", ";
      }

      int textWidth = extraText.length() * 6;
      int displayWidth = SCREEN_WIDTH;
      int xPos = displayWidth - scrollOffset;

      display.setCursor(xPos, 54); // bottom line
      display.print(extraText);

      if (millis() - lastScroll > scrollSpeed) {
        scrollOffset++;
        if (scrollOffset > textWidth + displayWidth) scrollOffset = 0;
        lastScroll = millis();
      }
    }

    display.display();
  }

  void setup() {
    Serial.begin(115200);
    Wire.begin(4,5);

    if (!display.begin(SSD1306_SWITCHCAPVCC,0x3C)) { for(;;); }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Smart School Bag");
    display.setCursor(0,20);
    display.println("Scan any book to start...");
    display.display();

    SPI.begin();
    mfrc522.PCD_Init();

    display.setCursor(0,40);
    display.println("Waiting first card...");
    display.display();
    while (!mfrc522.PICC_IsNewCardPresent()) { delay(50); }
    mfrc522.PICC_ReadCardSerial();
    byte* uid = mfrc522.uid.uidByte;
    unsigned long seed = ((unsigned long)uid[0]<<24)|((unsigned long)uid[1]<<16)|((unsigned long)uid[2]<<8)|uid[3];
    randomSeed(seed);
    currentDayIndex = random(0,5);
    Serial.print("Random day: "); Serial.println(dayNames[currentDayIndex]);
    updateDisplay();
    mfrc522.PICC_HaltA();
  }

  void loop() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte* uid = mfrc522.uid.uidByte;
      Serial.print("Scanned UID: ");
      for (byte i=0; i<mfrc522.uid.size; i++) { Serial.print(uid[i],HEX); Serial.print(" "); }
      Serial.println();

      bool recognized = false;
      for (int i=0; i<totalBooks; i++) {
        if (compareUID(uid, books[i].uid)) {
          recognized = true;
          if (!books[i].scanned) {
            books[i].scanned = true;
            Serial.print("Book Scanned: "); Serial.println(books[i].name);

            if (!isInTodaysTimetable(books[i].name) && !isAlreadyExtra(books[i].name)) {
              extraBooks[extraCount++] = books[i].name;
            }
          } else {
            Serial.println("Duplicate scan ignored");
          }
          break;
        }
      }

      if (!recognized) {
        if (!isAlreadyExtra("Unknown")) {
          extraBooks[extraCount++] = "Unknown";
        }
      }

      mfrc522.PICC_HaltA();
      updateDisplay();
    }
  }

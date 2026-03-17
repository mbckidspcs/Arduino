#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastScroll = 0;
int scrollOffset = 0;
const int scrollSpeed = 150; // ms per scroll step

String longText = "Extra: Science, Maths, English, History, Music, Tamil, Geography, Civic";

void setup() {
  Serial.begin(115200);
  Wire.begin(4,5); // SDA, SCL

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Monitored Subjects");

  // Normal text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,12);
  display.println("Normal Text - White");

  // Inverted text
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(0,24);
  display.println("Inverted Text");

  // Filled rectangle with black text
  display.fillRect(0,36,80,12,SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2,38);
  display.println("Black on White Box");

  display.display();
  delay(2000); // pause to view
}

void loop() {
  display.fillRect(0,50,128,14,SSD1306_BLACK); // clear previous scrolling area

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int textWidth = longText.length() * 6; // approx text width in pixels
  int xPos = SCREEN_WIDTH - scrollOffset;
  display.setCursor(xPos,50);
  display.print(longText);

  display.display();

  if(millis() - lastScroll > scrollSpeed){
    scrollOffset++;
    if(scrollOffset > textWidth + SCREEN_WIDTH) scrollOffset = 0;
    lastScroll = millis();
  }
}

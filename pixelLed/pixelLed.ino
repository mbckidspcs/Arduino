
#include <Adafruit_NeoPixel.h>

#define LED_PIN   5      // GPIO pin connected to NeoPixel DIN
#define NUM_LEDS  1      // Number of NeoPixel LEDs

// Create NeoPixel object
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ------------------- SETUP -------------------
void setup() {
  strip.begin();   // Initialize NeoPixel
  strip.show();    // Turn all pixels off
}

// ------------------- LOOP -------------------
void loop() {
  blinkColor(255, 0, 0, 5, 300);    // Red
  delay(1000);

  blinkColor(0, 255, 0, 5, 300);    // Green
  delay(1000);

  blinkColor(0, 0, 255, 5, 300);    // Blue
  delay(1000);

  blinkColor(255, 255, 0, 5, 300);  // Yellow
  delay(1000);

  blinkColor(0, 255, 255, 5, 300);  // Cyan
  delay(1000);

  blinkColor(255, 0, 255, 5, 300);  // Magenta
  delay(1000);

  blinkColor(255, 255, 255, 5, 300);// White
  delay(1000);
}

// ------------------- FUNCTIONS -------------------

// Set NeoPixel color
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

// Blink a given color
void blinkColor(uint8_t r, uint8_t g, uint8_t b, int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    setColor(r, g, b);   // LED ON
    delay(delayMs);
    setColor(0, 0, 0);   // LED OFF
    delay(delayMs);
  }
}
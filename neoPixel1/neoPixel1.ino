  #include <Adafruit_NeoPixel.h>

  #define LED_PIN   5     // ESP32 GPIO pin connected to NeoPixel DIN
  #define NUM_LEDS  50    // Number of NeoPixel LEDs
  #define WIDTH     4     // Width of the scanning bar (center + sides)

  Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

  int pos = 0;        // Current LED position
  int dir = 1;        // Direction: 1 = forward, -1 = backward

  void setup() {
    strip.begin();
    strip.show();
    strip.setBrightness(100); // Adjust overall brightness
  }

  void loop() {
    strip.clear();



    rainbowCycle(20);

    

    // Draw a "wide" rainbow bar
    for (int i = -WIDTH; i <= WIDTH; i++) {
      int index = pos + i;
      if (index >= 0 && index < NUM_LEDS) {
        // Fade effect: center is brightest, sides are dimmer
        int brightness = 255 - abs(i) * (255 / (WIDTH + 1));
        uint32_t color = dimColor(Wheel((index * 256 / NUM_LEDS) & 255), brightness);
        strip.setPixelColor(index, color);
      }
    }

    strip.show();
    delay(60);

    // Move the scanner
    pos += dir;
    if (pos >= NUM_LEDS - 1 || pos <= 0) {
      dir = -dir;  // Reverse at edges
    }
  }

  // Rainbow color wheel
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
      WheelPos -= 170;
      return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }

  // Adjust brightness of a color
  uint32_t dimColor(uint32_t color, int brightness) {
    uint8_t r = (uint8_t)(color >> 16);
    uint8_t g = (uint8_t)(color >> 8);
    uint8_t b = (uint8_t)(color);
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;
    return strip.Color(r, g, b);
  }


  // Rainbow cycle across all pixels
  void rainbowCycle(int wait) {
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
      for (int i = 0; i < strip.numPixels(); i++) {
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
      strip.show();
      delay(wait);
    }
  }

  
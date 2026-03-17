  #include <Keypad.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>

  // LCD
  LiquidCrystal_I2C lcd(0x27, 16, 2);

  // Ultrasonic pins
  #define TRIG_PIN 4
  #define ECHO_PIN 5

  // Keypad setup
  const byte ROWS = 4;
  const byte COLS = 4;
  char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
  };

  // Safe GPIOs (no conflicts)
  byte rowPins[ROWS] = {14, 27, 26, 25};
  byte colPins[COLS] = {33, 32, 23, 19};

  Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

  String weightInput = "";
  float weight = 0;
  float height = 0;
  float referenceHeight = 210; // cm
  float bmi = 0;

  int step = 0; // 0=weight, 1=live height, 2=BMI

  long readUltrasonic() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    long distance = duration * 0.034 / 2; // cm
    return distance;
  }

  void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("BMI Calculator");
    delay(2000);
    lcd.clear();
    lcd.print("Enter Weight(kg)");
  }

  void loop() {
    char key = keypad.getKey();

    if (step == 0) { // Enter weight
      if (key) {
        if (key >= '0' && key <= '9') {
          weightInput += key;
          lcd.setCursor(0,1);
          lcd.print(weightInput + " kg   ");
        } else if (key == '*') {
          weightInput = "";
          lcd.clear();
          lcd.print("Enter Weight(kg)");
        } else if (key == '#') {
          if (weightInput.length() > 0) {
            weight = weightInput.toFloat();
            step = 1; // move to live height measurement
            lcd.clear();
            lcd.print("Measuring Height");
            lcd.setCursor(0,1);
            lcd.print("Press # to confirm");
          }
        }
      }
    }
    else if (step == 1) { // Live height measurement
      long dist = readUltrasonic();  // distance from sensor to top of head
      height = (referenceHeight - dist) / 100.0; // convert to meters

      // Display live height
      lcd.setCursor(0,1);
      lcd.print("H: ");
      lcd.print(height,2);
      lcd.print(" m          "); // extra spaces to clear previous digits

      // Check if # is pressed to confirm height
      if (key == '#') {
        step = 2; // next step to calculate BMI
        lcd.clear();
        lcd.print("Height Confirmed");
        delay(1000);
      }
    }
    else if (step == 2) { // Calculate BMI
      bmi = weight / (height * height);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("BMI: ");
      lcd.print(bmi,1);
      lcd.setCursor(0,1);
      if (bmi < 18.5) lcd.print("Underweight");
      else if (bmi < 25) lcd.print("Normal");
      else if (bmi < 30) lcd.print("Overweight");
      else lcd.print("Obese");

      Serial.println("Weight: " + String(weight) + " kg");
      Serial.println("Height: " + String(height,2) + " m");
      Serial.println("BMI: " + String(bmi,2));

      step = 0; // reset
      weightInput = "";
      delay(5000);
      lcd.clear();
      lcd.print("Enter Weight(kg)");
    }
  }

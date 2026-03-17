  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <Servo.h>

  LiquidCrystal_I2C lcd(0x27, 16, 2);
  Servo gateServo;

  const int trigPin = 9;
  const int echoPin = 10;
  const int moisturePin = A0;
  const int servoPin = 11;

  // --- CALIBRATION VARIABLES ---
  int moistureThreshold = 200; // Change this after checking Serial Monitor
  int distanceThreshold = 15;  

  void setup() {
    Serial.begin(9600); // Initialize Serial Communication
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    lcd.init();
    lcd.backlight();
    gateServo.attach(servoPin);
    gateServo.write(88); 

    Serial.println("--- System Initializing ---");
    lcd.print("System Ready");
    delay(1000);
  }

  void loop() {
    long duration;
    int distance;
    int moistureValue;

    // Ultrasonic Ping
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    // Periodic debug print for distance
    if (millis() % 1000 == 0) { 
      Serial.print("Current Distance: ");
      Serial.print(distance);
      Serial.println(" cm");
    }
      Serial.print("Distance :");
      Serial.println(distance);
      
      lcd.setCursor(0, 0);
      lcd.print("System Ready...");
      
    if (distance > 0 && distance < distanceThreshold) {
      Serial.println("(!) Object Detected. Reading moisture...");
      lcd.setCursor(0, 0);
      lcd.print("Sensing Waste...");
      
      delay(2000); // Wait for waste to settle on sensor
      moistureValue = analogRead(moisturePin);

      // DEBUG: Critical for calibration
      Serial.print(">>> Raw Moisture Value: ");
      Serial.println(moistureValue);

      if (moistureValue > moistureThreshold) { 
        Serial.println("Result: WET WASTE");
        processWetWaste();
      } else {
        Serial.println("Result: DRY WASTE");
        processDryWaste();
      }
      
      delay(1000); // Time to clear the flap
      lcd.clear();
      gateServo.write(88); // Reset

      delay(500); // Time to clear the flap
    }
  }

  void processWetWaste() {
    lcd.clear();
    lcd.print("Type: WET");
    gateServo.write(40); 
  }

  void processDryWaste() {
    lcd.clear();
    lcd.print("Type: DRY");
    gateServo.write(145); 
  }
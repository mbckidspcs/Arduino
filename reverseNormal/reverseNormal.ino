  #define TRIG_PIN 5     // Ultrasonic Trigger
  #define ECHO_PIN 18    // Ultrasonic Echo
  #define BUZZER_PIN 23  // Buzzer pin

  int delay1 = 80;
  int delay2 = 100;
  int offdelay1 = 140;
  int offdelay2 = 1250;

   long lastDistanceTime = 0;
   long distanceInterval = 0; // 2 seconds
  long distance = 400;

  String serialBuffer = "";

  void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    Serial.println("Enter 4 delay values separated by space: delay1 delay2 offdelay1 offdelay2");
    Serial.println("Example: 30 30 100 200");
  }

  long getDistance() {
    // Clear trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Send 10us pulse
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure echo pulse
    long duration = pulseIn(ECHO_PIN, HIGH);
    long distance = duration * 0.034 / 2; // cm
    return distance;
  }

  void handleSerialInput() {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (serialBuffer.length() > 0) {
          // Split input by space
          int first = serialBuffer.indexOf(' ');
          int second = serialBuffer.indexOf(' ', first + 1);
          int third = serialBuffer.indexOf(' ', second + 1);

          if (first > 0 && second > first && third > second) {
            delay1 = serialBuffer.substring(0, first).toInt();
            delay2 = serialBuffer.substring(first + 1, second).toInt();
            offdelay1 = serialBuffer.substring(second + 1, third).toInt();
            offdelay2 = serialBuffer.substring(third + 1).toInt();

            Serial.println("✅ Delays updated:");
            Serial.print("delay1 = "); Serial.println(delay1);
            Serial.print("delay2 = "); Serial.println(delay2);
            Serial.print("offdelay1 = "); Serial.println(offdelay1);
            Serial.print("offdelay2 = "); Serial.println(offdelay2);
          } else {
            Serial.println("⚠️ Invalid input. Enter 4 numbers separated by space.");
          }
          serialBuffer = "";
        }
      } else {
        serialBuffer += c;
      }
    }
  }

  void loop() {
    handleSerialInput();

    if (millis() - lastDistanceTime >= distanceInterval) {
      lastDistanceTime = millis();

      distance = getDistance();
    }
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    int delayTime;
    distanceInterval = 0;

    if (distance < 40) {
      distanceInterval = 2500;
      normalAlarm(50 , 100);
      Serial.println(">> 1");
    } 
    else if (distance < 100) {
  
      normalAlarm(50, 300);
      Serial.println(">> 2");

    } 
    else if (distance < 160) {
  
      normalAlarm(100, 600);
      Serial.println(">> 3");

    }else {
      Serial.println(">> 4");
      farAlarm();
    }
  }

  void normalAlarm(int delay1,int delay2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delay1);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delay2);
  }

  void farAlarm() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delay1);
    digitalWrite(BUZZER_PIN, LOW);
    delay(offdelay1);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delay2);
    digitalWrite(BUZZER_PIN, LOW);
    delay(offdelay2);
  }

  void nearAlarm() {
    for(int i = 0; i < 5; i++){
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50 + (i * 5));
      digitalWrite(BUZZER_PIN, LOW);
      delay(50 + (i * 5));
    }
    delay(200);
  }

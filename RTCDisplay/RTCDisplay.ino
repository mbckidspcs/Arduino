  #include <Wire.h>
  #include "RTClib.h"        // For DS3231 RTC
  #include <LiquidCrystal_I2C.h> // For I2C LCD

  // ----------------- Pin Definitions -----------------
  #define BUZZER_PIN 14     // Buzzer connected to GPIO 14

  // ----------------- RTC and LCD Setup -----------------
  RTC_DS3231 rtc;
  LiquidCrystal_I2C lcd(0x27, 16, 2); // Change 0x27 to 0x3F if needed

  // ----------------- Alarm Time -----------------
   int ALARM_HOUR = 16;   // 4 PM in 24-hour format
   int ALARM_MIN = 15;    // 4:10 PM

  void setup() {
    Serial.begin(115200);
  
    // Buzzer as output
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially

    // Start I2C LCD
    lcd.init();
    lcd.backlight();

    // Start RTC
    if (!rtc.begin()) {
      Serial.println("Couldn't find RTC");
      lcd.setCursor(0,0);
      lcd.print("RTC NOT FOUND!");
      while (1);
    }

    // ----------------- Set RTC to current Sri Lanka time -----------------
    // Change these values to the current Sri Lanka date & time
    // DateTime(year, month, day, hour, minute, second)
    //rtc.adjust(DateTime(2026, 1, 14, 15, 0, 0)); // Example: 14 Jan 2026, 15:00:00 Sri Lanka time

      DateTime now1 = rtc.now();
    ALARM_HOUR = now1.hour();
    ALARM_MIN = now1.minute() + 1;
    

  }

  void loop() {
    DateTime now = rtc.now();

    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();

    // ----------------- Display on LCD -----------------
    lcd.setCursor(0,0);
    lcd.print("Time: ");
    if(hour < 10) lcd.print('0');
    lcd.print(hour);
    lcd.print(':');
    if(minute < 10) lcd.print('0');
    lcd.print(minute);
    lcd.print(':');
    if(second < 10) lcd.print('0');
    lcd.print(second);


    lcd.setCursor(0,1);
    lcd.print("Alarm: ");
    if(ALARM_HOUR < 10) lcd.print('0');
    lcd.print(ALARM_HOUR);
    lcd.print(':');
    if(ALARM_MIN < 10) lcd.print('0');
    lcd.print(ALARM_MIN);

    // ----------------- Alarm -----------------
    if(hour == ALARM_HOUR && minute == ALARM_MIN && second < 10) {
      digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
    } else {
      digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    }

    delay(1000); 
  }
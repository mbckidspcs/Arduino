    #include <Arduino.h>
    #include <Preferences.h>

    #define TRIG_PIN   5
    #define ECHO_PIN   18
    #define BUZZER_PIN 23

    Preferences prefs;
    int farDelay;
    int nearDelay;

    // ---------------------- Ultrasonic ----------------------
    long getDistance() {
      digitalWrite(TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);
      long duration = pulseIn(ECHO_PIN, HIGH, 30000);
      long distance = duration * 0.034 / 2; // cm
      return distance;
    }

    // ---------------------- Basic Tone Generator ----------------------
    void playTone(int freq, int duration) {
      tone(BUZZER_PIN, freq, duration);
      delay(duration + 20);
    }

    void playToneNormal() {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
      delay(nearDelay);
    }

    // ---------------------- iPhone Signal Tone ----------------------
    void iphoneSignalTone() {
      playTone(880, 150);   // A5
      playTone(988, 150);   // B5
      playTone(1175, 150);  // D6
      playTone(1319, 150);  // E6
      delay(nearDelay);
    }

    // ---------------------- 5 Reverse Alarm Tones ----------------------
    void fastBeepTone() {
      for (int i = 0; i < 5; i++) {
        playTone(1200, 100);
        delay(50);
      }
    }

    void slowBeepTone() {
      for (int i = 0; i < 3; i++) {
        playTone(800, 300);
        delay(50);
      }
    }

    void ascendingTone() {
      int freqs[] = {500, 700, 900, 1100, 1300};
      for (int i = 0; i < 5; i++) playTone(freqs[i], 150);
    }

    void descendingTone() {
      int freqs[] = {1300, 1100, 900, 700, 500};
      for (int i = 0; i < 5; i++) playTone(freqs[i], 150);
    }

    void warblingTone() {
      for (int i = 0; i < 5; i++) {
        playTone(1000, 150);
        playTone(700, 150);
      }
    }

    void reverseAlarmTone() {
    playTone(2500, 400);
    delay(farDelay);
  }

    // ---------------------- Setup ----------------------
    void setup() {
      Serial.begin(115200);
      pinMode(TRIG_PIN, OUTPUT);
      pinMode(ECHO_PIN, INPUT);
      pinMode(BUZZER_PIN, OUTPUT);

      prefs.begin("reverse", false);
      farDelay = prefs.getInt("farDelay", 600);
      nearDelay = prefs.getInt("nearDelay", 200);

      Serial.println("Delays loaded:");
      Serial.print("farDelay = "); Serial.println(farDelay);
      Serial.print("nearDelay = "); Serial.println(nearDelay);
    }

    // ---------------------- Main Loop ----------------------
    void loop() {
      long distance = getDistance();
      Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");

      if (distance > 0 && distance < 60) {
        
        playToneNormal();
      } 
      else if (distance >= 60 && distance < 100) {
        iphoneSignalTone();
        delay(farDelay);
      } 
      else if (distance >= 80 && distance < 180) {
        slowBeepTone();
        delay(farDelay);
      } 
      else if (distance >= 100 && distance < 250) {
        ascendingTone();
        delay(farDelay);
      } 
      else if (distance >= 120 && distance < 320) {
        descendingTone();
        delay(farDelay);
      } 
      else if (distance >= 140 && distance < 400) {
        warblingTone();
        delay(farDelay);
      } 
      else {
        reverseAlarmTone();
      }
    }

    // ---------------------- Optional: Update delays ----------------------
    void saveNewDelays(int newFar, int newNear) {
      prefs.putInt("farDelay", newFar);
      prefs.putInt("nearDelay", newNear);
      Serial.println("Delays updated in memory!");
    }



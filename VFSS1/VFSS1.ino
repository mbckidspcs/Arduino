
  #include <TinyGPS++.h>

  // Pin Definitions
  const int flamePins[3] = {32, 33, 25};     // Flame sensors
  const int relayPins[3] = {18, 19, 21};     // Relays for each flame sensor
  const int relay4Pin = 22;                  // Master relay
  const int smokePin = 34;                   // Analog pin for MQ-135
  const int buzzerPin = 23;                  // Buzzer

  // Threshold for smoke detection
  const int smokeThreshold = 800; // Adjust based on your environment

  // GPS & GSM Setup
  HardwareSerial gpsSerial(1);  // Use UART1
  HardwareSerial gsmSerial(2); // Use UART2

  TinyGPSPlus gps;

  // Phone number to send SMS
  String phoneNumber1 = "+94703927827"; // Replace with actual number
  String phoneNumber2 = "+94783964519"; // Replace with actual number

  void setup() {
    
    Serial.begin(115200);
    gpsSerial.begin(9600);
    gsmSerial.begin(9600);

    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // TX=17, RX=16
    gsmSerial.begin(9600, SERIAL_8N1, 27, 26);  // TX=26, RX=27



    // Setup pins
    for (int i = 0; i < 3; i++) {
      pinMode(flamePins[i], INPUT);
      pinMode(relayPins[i], OUTPUT);
      digitalWrite(relayPins[i], LOW);
    }

    pinMode(relay4Pin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    digitalWrite(relay4Pin, LOW);
    digitalWrite(buzzerPin, LOW);

    // Initialize GSM
    delay(1000);
    gsmSerial.println("AT");
    delay(1000);
    gsmSerial.println("AT+CMGF=1"); // SMS Text Mode
    delay(1000);
  }

  void loop() {
    // Read GPS data continuously
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }

    bool fireDetected = false;

    // Check flame sensors
    for (int i = 0; i < 3; i++) {
      if (digitalRead(flamePins[i]) == LOW) { // Flame detected
        fireDetected = true;
        digitalWrite(relayPins[i], HIGH);

        Serial.print("Relay on:" );
        Serial.println(relayPins[i]);

        triggerAlert(" Fire detected at sensor " + String(i + 1));


        delay(2000);
        digitalWrite(relayPins[i], LOW);
      }
    }

    // Check smoke sensor
    int smokeValue = analogRead(smokePin);
    
      Serial.print("Smoke Sensor Value : ");
      Serial.println(smokeValue);

    if (smokeValue > smokeThreshold) {
      fireDetected = true;
      digitalWrite(buzzerPin, HIGH);
      triggerAlert(" Smoke detected! Value: " + String(smokeValue));
      delay(2000);
      digitalWrite(buzzerPin, LOW);
    }

    delay(1000);
  }


  void triggerAlert(String reason) {

    digitalWrite(relay4Pin, HIGH); // Trigger master relay

    String location = getGPSLocation();
    String message = reason + "\nLocation: " + location;

    sendSMS(message,phoneNumber1);
    sendSMS(message,phoneNumber2);

    delay(5000);

    digitalWrite(relay4Pin, LOW);
  }


  String getGPSLocation() {
    if (gps.location.isValid()) {
      return "https://maps.google.com/?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    } else {
      return "GPS not fixed.";
    }
  }

  void sendSMS(String msg,String phoneNumber) {

      Serial.print("Sending SMS :" );
      Serial.println(msg);

      gsmSerial.println("AT+CMGS=\"" + phoneNumber + "\"");

      delay(1000);
      gsmSerial.print(msg);
      delay(1000);
      gsmSerial.write(26); // Ctrl+Z to send
      delay(2000);

  }


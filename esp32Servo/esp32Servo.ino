#include <ESP32Servo.h>

Servo myServo;  // create servo object

int servoPin = 18;  // GPIO pin connected to servo signal
int angle = 0;

void setup() {
  Serial.begin(115200);

  // Attach servo to pin
  myServo.attach(servoPin);

  Serial.println("Servo test start...");
}

void loop() {
  // Sweep from 0 to 180
  for (angle = 0; angle <= 180; angle++) {
    myServo.write(angle);
    delay(15);  // delay for smooth movement
  }

  // Sweep back from 180 to 0
  for (angle = 180; angle >= 0; angle--) {
    myServo.write(angle);
    delay(15);
  }
}

#include <Servo.h>

Servo ch1;

void setup() {
  ch1.attach(9);          // PWM output pin
}

void loop() {
  for (int us = 1000; us <= 2000; us += 10) {
    ch1.writeMicroseconds(us);
    delay(20);           // 50Hz frame
  }

  for (int us = 2000; us >= 1000; us -= 10) {
    ch1.writeMicroseconds(us);
    delay(20);
  }
}

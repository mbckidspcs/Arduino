void setup() {
  pinMode(2, OUTPUT); // Onboard LED is typically on GPIO 2
}

void loop() {
  digitalWrite(2, HIGH); // Turn the LED ON
  delay(1000);           // Wait for 1 second
  digitalWrite(2, LOW);  // Turn the LED OFF
  delay(1000);           // Wait for 1 second
}

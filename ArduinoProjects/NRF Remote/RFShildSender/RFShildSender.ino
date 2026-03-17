#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pins

const byte address[6] = "00001";

struct DataPackage {
  int16_t x;
  int16_t y;
  uint8_t buttons;
};

DataPackage data;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();

  Serial.println("Transmitter ready");
}

void loop() {
  data.x = analogRead(A0); // X-axis
  data.y = analogRead(A1); // Y-axis
  data.buttons = 0;

  // Read buttons on joystick shield
  if (!digitalRead(2)) data.buttons |= 0x01;
  if (!digitalRead(3)) data.buttons |= 0x02;
  if (!digitalRead(4)) data.buttons |= 0x04;
  if (!digitalRead(5)) data.buttons |= 0x08;
  if (!digitalRead(6)) data.buttons |= 0x10;
  if (!digitalRead(7)) data.buttons |= 0x20;

  radio.write(&data, sizeof(DataPackage));

  Serial.print("X: ");
  Serial.print(data.x);
  Serial.print(" | Y: ");
  Serial.print(data.y);
  Serial.print(" | Btns: ");
  Serial.println(data.buttons, BIN);

  delay(50);
}

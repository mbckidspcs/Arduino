#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- GPS -----------------
TinyGPSPlus gps;
HardwareSerial GPS_Serial(1);   // UART1

// ---------------- SIM800L ----------------
HardwareSerial SIM800(2);       // UART2
String phoneNumber = "+94755991832";   // CHANGE NUMBER

double latitude = 0.0;
double longitude = 0.0;

void sendSMS(double lat, double lng);

void setup() {
  Serial.begin(115200);

  // OLED init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("GPS Tracker");
  display.println("Initializing...");
  display.display();

  // GPS (RX, TX)
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17);

  // SIM800L (RX, TX)
  SIM800.begin(9600, SERIAL_8N1, 26, 27);

  delay(3000);

  // SIM800 SMS mode
  SIM800.println("AT");
  delay(1000);
  SIM800.println("AT+CMGF=1"); // Text mode
  delay(1000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Waiting GPS Fix...");
  display.display();
}

void loop() {

  // Read GPS data
  while (GPS_Serial.available()) {
    gps.encode(GPS_Serial.read());
  }

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    // Display on OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("GPS LOCATION");
    display.println("----------------");
    display.print("Lat: ");
    display.println(latitude, 6);
    display.print("Lng: ");
    display.println(longitude, 6);
    display.display();

    // Send SMS once GPS is valid
    sendSMS(latitude, longitude);
    delay(30000);   // Send SMS every 30 sec
  }
}

void sendSMS(double lat, double lng) {
  String message = "GPS Location:\n";
  message += "Lat: " + String(lat, 6) + "\n";
  message += "Lng: " + String(lng, 6) + "\n";
  message += "Map: https://maps.google.com/?q=";
  message += String(lat, 6) + "," + String(lng, 6);

  SIM800.print("AT+CMGS=\"");
  SIM800.print(phoneNumber);
  SIM800.println("\"");
  delay(1000);
  Serial.print("Sending SMS :");
  Serial.println(message);

  SIM800.print(message);

  delay(500);
  SIM800.write(26);  // CTRL+Z
  delay(5000);
}

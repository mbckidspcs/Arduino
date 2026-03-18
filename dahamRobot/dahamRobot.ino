#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Configuration ---
const char* ssid = "Robot_Car_AP";
const char* password = "password123";

// ESP32 38-pin I2C default pins
#define I2C_SDA 21
#define I2C_SCL 22

// Try 0x27 or 0x3F if 0x27 fails
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 4); 

Servo leftServo;
Servo rightServo;

// Motor Driver Pins (L298N)
const int IN1 = 27; const int IN2 = 26; // Left Motor
const int IN3 = 25; const int IN4 = 33; // Right Motor
const int ENA = 14; const int ENB = 32; // Speed Pins (PWM)

// Servo "Down" Positions
const int LEFT_DOWN = 0;
const int RIGHT_DOWN = 180;

// State Variables
int currentSpeed = 150; 
String currentDir = "STOP";
bool isMoving = false;

// --- HTML Web Page ---
const char* htmlPage = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: 'Segoe UI', sans-serif; text-align: center; background: #1a1a1a; color: white; margin: 0; padding: 20px; }
  .grid { display: grid; grid-template-columns: repeat(3, 80px); justify-content: center; gap: 15px; margin-top: 20px; }
  .btn { width: 80px; height: 80px; border-radius: 15px; border: none; font-weight: bold; cursor: pointer; transition: 0.2s; user-select: none; -webkit-tap-highlight-color: transparent; }
  .move { background: #3498db; color: white; }
  .stop { background: #e74c3c; color: white; grid-column: 2; }
  .btn:active { transform: scale(0.9); background: #2ecc71; }
  .slider-container { margin: 40px auto; width: 90%; max-width: 400px; background: #2c3e50; padding: 20px; border-radius: 15px; }
  input[type=range] { width: 100%; height: 10px; border-radius: 5px; background: #555; outline: none; -webkit-appearance: none; }
  input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 30px; height: 30px; border-radius: 50%; background: #2ecc71; cursor: pointer; border: 3px solid white; }
</style></head>
<body>
  <h2>ROBOT COMMAND</h2>
  <div class="grid">
    <button class="btn move" style="grid-column: 2" onmousedown="send('forward')" onmouseup="send('stop')" ontouchstart="send('forward')" ontouchend="send('stop')">UP</button>
    <button class="btn move" onmousedown="send('left')" onmouseup="send('stop')" ontouchstart="send('left')" ontouchend="send('stop')">LEFT</button>
    <button class="btn stop" onclick="send('stop')">STOP</button>
    <button class="btn move" onmousedown="send('right')" onmouseup="send('stop')" ontouchstart="send('right')" ontouchend="send('stop')">RIGHT</button>
    <button class="btn move" style="grid-column: 2" onmousedown="send('backward')" onmouseup="send('stop')" ontouchstart="send('backward')" ontouchend="send('stop')">DOWN</button>
  </div>
  <div class="slider-container">
    <p>SPEED: <span id="val">150</span></p>
    <input type="range" min="0" max="255" value="150" oninput="updateSpeed(this.value)">
  </div>
  <script>
    function send(dir) { fetch('/' + dir); }
    function updateSpeed(s) { document.getElementById('val').innerText = s; fetch('/speed?v=' + s); }
  </script>
</body></html>)rawliteral";

// --- LCD and Utility Functions ---

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print("DIR  : " + currentDir + "      ");
  lcd.setCursor(0, 2);
  lcd.print("SPEED: " + String(currentSpeed) + "  / 255 ");
}

void applyMotors(int s1, int s2, int s3, int s4) {
  digitalWrite(IN1, s1); digitalWrite(IN2, s2);
  digitalWrite(IN3, s3); digitalWrite(IN4, s4);
  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

void scanI2C() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning I2C...");
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found\n");
}

// --- Movement Functions ---

void robotForward() {
  currentDir = "FORWARD";
  isMoving = true;
  applyMotors(HIGH, LOW, HIGH, LOW);
  updateLCD();
  Serial.println("Robot: Moving Forward");
}

void robotBackward() {
  currentDir = "BACKWARD";
  isMoving = true;
  applyMotors(LOW, HIGH, LOW, HIGH);
  updateLCD();
  Serial.println("Robot: Moving Backward");
}

void robotLeft() {
  currentDir = "LEFT";
  isMoving = true;
  applyMotors(LOW, HIGH, HIGH, LOW);
  updateLCD();
  Serial.println("Robot: Turning Left");
}

void robotRight() {
  currentDir = "RIGHT";
  isMoving = true;
  applyMotors(HIGH, LOW, LOW, HIGH);
  updateLCD();
  Serial.println("Robot: Turning Right");
}

void robotStop() {
  currentDir = "STOP";
  isMoving = false;
  applyMotors(LOW, LOW, LOW, LOW);
  updateLCD();
  Serial.println("Robot: Stopped");
}

// --- Setup and Loop ---

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Robot Booting ---");
  
  // Explicitly start I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  scanI2C();

  // LCD Init
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Robot Initializing");

  // Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  // Servos - Set to DOWN position immediately
  leftServo.attach(13); 
  rightServo.attach(12);
 // leftServo.write(LEFT_DOWN);
//  rightServo.write(RIGHT_DOWN);
  Serial.println("Servos Initialized to DOWN position");

  // WiFi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SSID: " + String(ssid));
  lcd.setCursor(0, 3); lcd.print("IP  : 192.168.4.1");
  
  Serial.print("AP Started. SSID: "); Serial.println(ssid);
  Serial.print("Web Server IP: "); Serial.println(IP);

  // Web Routes
  server.on("/", []() { 
    server.send(200, "text/html", htmlPage); 
    Serial.println("Web: Interface loaded");
  });
  
  server.on("/forward", []() { robotForward(); server.send(200); });
  server.on("/backward", []() { robotBackward(); server.send(200); });
  server.on("/left", []() { robotLeft(); server.send(200); });
  server.on("/right", []() { robotRight(); server.send(200); });
  server.on("/stop", []() { robotStop(); server.send(200); });
  
  server.on("/speed", []() {
    if (server.hasArg("v")) {
      currentSpeed = server.arg("v").toInt();
      Serial.print("Speed Update: "); Serial.println(currentSpeed);
      if (isMoving) applyMotors(digitalRead(IN1), digitalRead(IN2), digitalRead(IN3), digitalRead(IN4));
      updateLCD();
    }
    server.send(200);
  });

  server.begin();
  updateLCD();
  Serial.println("HTTP Server Ready");
}

void animateHands() {
  static unsigned long lastMove = 0;
  static int angle = 0;
  static bool dirUp = true;
  
  if (millis() - lastMove > 30) {
    if (dirUp) angle += 2; else angle -= 2;
    if (angle >= 40 || angle <= 0) dirUp = !dirUp;
    
    leftServo.write(angle);
    rightServo.write(angle);
    lastMove = millis();
  }
}

void loop() {
  server.handleClient();
  if (isMoving && currentDir == "FORWARD") {
    animateHands();
  } else {
    // Keep hands in DOWN position when not moving forward
  //  leftServo.write(LEFT_DOWN);
   // rightServo.write(RIGHT_DOWN);
  }
}
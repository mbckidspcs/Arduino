#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Configuration ---
const char* ssid = "Robot_Car_AP";
const char* password = "12345678";

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
String customLCDText = "";

// --- HTML Web Page (Modern Arrow Layout) ---
const char* htmlPage = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: 'Segoe UI', sans-serif; text-align: center; background: #0f0f0f; color: #e0e0e0; margin: 0; padding: 20px; }
  .controller { display: grid; grid-template-columns: repeat(3, 80px); grid-template-rows: repeat(3, 80px); justify-content: center; gap: 15px; margin: 30px auto; }
  .btn { background: #222; color: #3498db; border: 2px solid #3498db; border-radius: 15px; font-size: 32px; cursor: pointer; transition: 0.2s; display: flex; align-items: center; justify-content: center; user-select: none; -webkit-tap-highlight-color: transparent; }
  .btn:active { background: #3498db; color: #fff; transform: scale(0.9); box-shadow: 0 0 20px #3498db; }
  .stop-btn { background: #c0392b; border-color: #e74c3c; color: white; grid-column: 2; grid-row: 2; font-size: 20px; font-weight: bold; }
  .stop-btn:active { background: #e74c3c; box-shadow: 0 0 20px #e74c3c; }
  
  .slider-card, .text-card { background: #1a1a1a; padding: 20px; border-radius: 20px; max-width: 400px; margin: 20px auto; border: 1px solid #333; }
  input[type=range] { width: 100%; height: 8px; border-radius: 5px; background: #333; outline: none; -webkit-appearance: none; margin: 15px 0; }
  input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 25px; height: 25px; border-radius: 50%; background: #3498db; cursor: pointer; border: 3px solid #fff; }
  
  .input-group { display: flex; gap: 10px; margin-top: 10px; }
  input[type=text] { flex: 1; padding: 10px; border-radius: 8px; border: 1px solid #444; background: #000; color: #3498db; outline: none; }
  .send-btn { padding: 10px 20px; background: #3498db; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; }
</style></head>
<body>
  <h2 style="color: #3498db; letter-spacing: 2px; margin-bottom: 5px;">NEO TECH VISION</h2>
  <p style="color: #666; font-size: 12px; margin: 0;">REMOTE CONTROLLER</p>
  
  <div class="controller">
    <!-- UP -->
    <button class="btn" style="grid-column: 2; grid-row: 1;" onmousedown="send('forward')" onmouseup="send('stop')" ontouchstart="send('forward')" ontouchend="send('stop')">&#129145;</button>
    
    <!-- LEFT -->
    <button class="btn" style="grid-column: 1; grid-row: 2;" onmousedown="send('left')" onmouseup="send('stop')" ontouchstart="send('left')" ontouchend="send('stop')">&#129144;</button>
    
    <!-- STOP -->
    <button class="btn stop-btn" onclick="send('stop')">STOP</button>
    
    <!-- RIGHT -->
    <button class="btn" style="grid-column: 3; grid-row: 2;" onmousedown="send('right')" onmouseup="send('stop')" ontouchstart="send('right')" ontouchend="send('stop')">&#129146;</button>
    
    <!-- DOWN -->
    <button class="btn" style="grid-column: 2; grid-row: 3;" onmousedown="send('backward')" onmouseup="send('stop')" ontouchstart="send('backward')" ontouchend="send('stop')">&#129147;</button>
  </div>

  <div class="slider-card">
    <p style="margin:0;">ENGINE POWER: <span id="val" style="color:#3498db; font-weight:bold;">150</span></p>
    <input type="range" min="0" max="255" value="150" oninput="updateSpeed(this.value)">
  </div>

  <div class="text-card">
    <p style="margin:0 0 10px 0; font-size: 14px; color: #888;">LCD ROW 4 MESSAGE</p>
    <div class="input-group">
      <input type="text" id="lcdMsg" placeholder="Type here..." maxlength="16">
      <button class="send-btn" onclick="sendText()">SEND</button>
    </div>
  </div>

  <script>
    function send(dir) { fetch('/' + dir); }
    function updateSpeed(s) { document.getElementById('val').innerText = s; fetch('/speed?v=' + s); }
    function sendText() { 
      let msg = document.getElementById('lcdMsg').value;
      fetch('/msg?t=' + encodeURIComponent(msg));
    }
  </script>
</body></html>)rawliteral";

// --- LCD and Utility Functions ---

void initiatDisplay(){
  lcd.clear();
  lcd.setCursor(1, 0); lcd.print("NAMO BUDDHAYA!");
  lcd.setCursor(3, 1); lcd.print("WELCOME TO");
  lcd.setCursor(-3, 2); lcd.print("NEO TECH VISION");
}

void applyMotors(int s1, int s2, int s3, int s4,int speed) {
  digitalWrite(IN1, s1); digitalWrite(IN2, s2);
  digitalWrite(IN3, s3); digitalWrite(IN4, s4);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
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
}

// --- Movement Functions ---

void robotForward() {
  initiatDisplay();
  currentDir = "FORWARD";
  isMoving = true;
  applyMotors(HIGH, LOW, HIGH, LOW,currentSpeed);
  Serial.println("Robot: Moving Forward");
}

void robotBackward() {
  lcd.clear();
  lcd.setCursor(3, 1); lcd.print("BYE !!!");
  currentDir = "BACKWARD";
  isMoving = true;
  applyMotors(LOW, HIGH, LOW, HIGH,currentSpeed);
  Serial.println("Robot: Moving Backward");
}

void robotRight() {
  currentDir = "LEFT";
  isMoving = true;
  applyMotors(LOW, HIGH, HIGH, LOW,currentSpeed+20);
  Serial.println("Robot: Turning Left");
}

void robotLeft() {
  currentDir = "RIGHT";
  isMoving = true;
  applyMotors(HIGH, LOW, LOW, HIGH,currentSpeed+20);
  Serial.println("Robot: Turning Right");
}

void robotStop() {
  currentDir = "STOP";
  isMoving = false;
  applyMotors(LOW, LOW, LOW, LOW, currentSpeed);
  Serial.println("Robot: Stopped");
}

// --- Setup and Loop ---

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  scanI2C();

  lcd.init(); 
  lcd.backlight();
  initiatDisplay();
  
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  leftServo.attach(13); 
  rightServo.attach(12);

  WiFi.softAP(ssid, password);
  
  server.on("/", []() { 
    server.send(200, "text/html", htmlPage); 
  });
  
  server.on("/forward", []() { robotForward(); server.send(200); });
  server.on("/backward", []() { robotBackward(); server.send(200); });
  server.on("/left", []() { robotLeft(); server.send(200); });
  server.on("/right", []() { robotRight(); server.send(200); });
  server.on("/stop", []() { robotStop(); server.send(200); });
  
  server.on("/speed", []() {
    if (server.hasArg("v")) {
      currentSpeed = server.arg("v").toInt();
      if (isMoving) applyMotors(digitalRead(IN1), digitalRead(IN2), digitalRead(IN3), digitalRead(IN4),currentSpeed);
    }
    server.send(200);
  });

  server.on("/msg", []() {
    if (server.hasArg("t")) {
      customLCDText = server.arg("t");
      lcd.clear();
      lcd.setCursor(-3, 1); 
      lcd.print("                ");
      lcd.setCursor(-3, 1); // Changed from -3 offset to 0 for standard printing
      lcd.print(customLCDText);
    }
    server.send(200);
  });

  server.begin();
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
  }
}
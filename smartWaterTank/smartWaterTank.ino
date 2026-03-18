#include <WiFi.h>
#include <WebServer.h>

// --- Configuration ---
const char* ap_ssid = "Smart-Tank-System";
const char* ap_pass = "12345678";

#define TRIG_PIN 5
#define ECHO_PIN 18
#define RELAY_PUMP 14
#define RELAY_SOLENOID 27
#define BUZZER_PIN 13

#define TANK_HEIGHT 20.0  
#define TANK_DIAMETER 10.0

// --- State Variables ---
bool isAutoMode = true;
bool pumpState = false;
bool solenoidState = false;
String historyLog = "System Started";

WebServer server(80);

float getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return (duration > 0) ? (duration * 0.034 / 2) : TANK_HEIGHT;
}

// --- HTML Interface ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body { font-family: sans-serif; text-align: center; background: #f0f2f5; color: #333; }
  .card { background: white; padding: 20px; border-radius: 15px; display: inline-block; width: 90%; max-width: 400px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); margin-top: 20px; }
  .tank { width: 120px; height: 180px; border: 4px solid #444; margin: 15px auto; position: relative; border-radius: 0 0 15px 15px; background: #eee; overflow: hidden; }
  .water { position: absolute; bottom: 0; width: 100%; background: linear-gradient(to top, #2980b9, #3498db); transition: 0.8s ease; }
  .btn { padding: 12px 20px; margin: 8px; border: none; border-radius: 8px; color: white; cursor: pointer; font-weight: bold; width: 80%; }
  .on { background: #27ae60; } .off { background: #95a5a6; } .auto { background: #2980b9; } .manual { background: #e67e22; }
  .log-box { text-align: left; font-size: 0.85rem; background: #333; color: #0f0; padding: 10px; height: 80px; overflow-y: auto; margin-top: 15px; border-radius: 5px; }
</style>
</head>
<body>
  <div class="card">
    <h2>Tank Control Panel</h2>
    <div class="tank"><div id="water" class="water" style="height:0%"></div></div>
    <p><b>Level:</b> <span id="lvl">0</span> cm | <b>Volume:</b> <span id="vol">0</span> ml</p>
    <hr>
    <button id="mBtn" class="btn" onclick="send('/set?mode=toggle')">AUTO MODE</button>
    <button id="pBtn" class="btn" onclick="send('/set?pump=toggle')">PUMP: OFF</button>
    <button id="sBtn" class="btn" onclick="send('/set?sole=toggle')">SOLENOID: OFF</button>
    <div class="log-box" id="log">System Booting...</div>
  </div>
<script>
function send(path) { fetch(path); }
setInterval(() => {
  fetch('/status').then(r => r.json()).then(d => {
    document.getElementById('water').style.height = d.pct + "%";
    document.getElementById('lvl').innerText = d.lvl;
    document.getElementById('vol').innerText = d.vol;
    document.getElementById('mBtn').innerText = d.auto ? "AUTO MODE" : "MANUAL MODE";
    document.getElementById('mBtn').className = "btn " + (d.auto ? "auto" : "manual");
    document.getElementById('pBtn').innerText = "PUMP: " + (d.pump ? "ON" : "OFF");
    document.getElementById('pBtn').className = "btn " + (d.pump ? "on" : "off");
    document.getElementById('sBtn').innerText = "SOLENOID: " + (d.sole ? "ON" : "OFF");
    document.getElementById('sBtn').className = "btn " + (d.sole ? "on" : "off");
    document.getElementById('log').innerHTML = d.log;
  });
}, 1000);
</script>
</body></html>)rawliteral";

// --- Server Handlers ---
void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleStatus() {
  float d = getDistance();
  float h = constrain(TANK_HEIGHT - d, 0, TANK_HEIGHT);
  int pct = (h / TANK_HEIGHT) * 100;
  int vol = 3.14159 * 25 * h; // r=5cm -> Area=78.5

  String json = "{";
  json += "\"lvl\":" + String(h, 1) + ",";
  json += "\"vol\":" + String(vol) + ",";
  json += "\"pct\":" + String(pct) + ",";
  json += "\"auto\":" + String(isAutoMode) + ",";
  json += "\"pump\":" + String(pumpState) + ",";
  json += "\"sole\":" + String(solenoidState) + ",";
  json += "\"log\":\"" + historyLog + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSet() {
  if (server.hasArg("mode")) {
    isAutoMode = !isAutoMode;
    historyLog = (isAutoMode ? "Mode set to AUTO" : "Mode set to MANUAL") + String("<br>") + historyLog;
  }
  if (!isAutoMode) {
    if (server.hasArg("pump")) {
      pumpState = !pumpState;
      historyLog = (pumpState ? "Manual Pump ON" : "Manual Pump OFF") + String("<br>") + historyLog;
    }
  }
  if (server.hasArg("sole")) {
    solenoidState = !solenoidState;
    historyLog = (solenoidState ? "Solenoid OPEN" : "Solenoid CLOSED") + String("<br>") + historyLog;
  }
  server.send(200);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PUMP, OUTPUT); pinMode(RELAY_SOLENOID, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PUMP, HIGH); digitalWrite(RELAY_SOLENOID, HIGH);

  WiFi.softAP(ap_ssid, ap_pass);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();

  if (isAutoMode) {
    float h = TANK_HEIGHT - getDistance();
    if (h < 5.0 && !pumpState) {
      pumpState = true;
      historyLog = "Auto: Low Level - Pump ON<br>" + historyLog;
    } else if (h > 17.0 && pumpState) {
      pumpState = false;
      historyLog = "Auto: Tank Full - Pump OFF<br>" + historyLog;
    }
  }

  digitalWrite(RELAY_PUMP, pumpState ? LOW : HIGH);
  digitalWrite(RELAY_SOLENOID, solenoidState ? LOW : HIGH);
}
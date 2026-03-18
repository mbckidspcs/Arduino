// Define pins for Ultrasonic Sensor
const int trigPin = 9;
const int echoPin = 10;

// Define pins for L298N Motor Driver (Motor A)
const int in1 = 7;
const int in2 = 6;
const int enA = 5;

// Variables for distance calculation
long duration;
int distance;
int safetyThreshold = 20; // Distance in cm to trigger motor

void setup() {
  // Motor pins as outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);
  
  // Sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600); // For debugging
}

void loop() {
  // 1. Get distance from sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Formula: distance = (time * speed of sound) / 2

  // 2. Logic: If object is detected within threshold, turn on motor
  if (distance <= safetyThreshold && distance > 0) {
    Serial.println("Object Detected! Motor ON");
    
    digitalWrite(in1, LOW); // Spin Direction Forward
    digitalWrite(in2, HIGH);
    analogWrite(enA, 200);   // Speed (0-255)
  } 
  else {
    Serial.println("Path Clear. Motor OFF");
    
    digitalWrite(in1, HIGH);  // Stop Motor
    digitalWrite(in2, LOW);
    analogWrite(enA, 0);
  }

  delay(100); // Small delay for stability
}
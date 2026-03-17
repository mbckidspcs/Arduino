  #include "BluetoothSerial.h"
  #include <Arduino.h>
  #include <Stepper.h>
  #include <ESP32Servo.h>   // ESP32-compatible servo library


  #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to enable it
  #endif

  // Create an instance of the BluetoothSerial object
  BluetoothSerial SerialBT;

  // String to hold the incoming command
  String command = "";
  bool stepperOn = false;
  int stepperSpeed = 0;
 
  // ================== L298N Motor Driver Pins ==================
  #define ENA 14   // Enable A (PWM Left Motor)
  #define dcMotor1Pin1 27
  #define dcMotor1Pin2 26
  #define dcMotor2Pin1 25
  #define dcMotor2Pin2 33
  #define ENB 32   // Enable B (PWM Right Motor)

  // ================== Stepper Motor Pins ==================
  #define STEPPER_IN1 19
  #define STEPPER_IN2 18
  #define STEPPER_IN3 5
  #define STEPPER_IN4 17

  // ================== Servo Pins ==================
  #define servoRotaryPin 21
  #define servoDiggingPin 22
  #define servoCoveringPin 23

  // ================== Relay Pins ==================
  #define rotoryMotorRelayPin 4
  #define waterPumpRelayPin 16   // second relay

  // ================== Globals ==================
  int motorSpeed = 200; // 0–255 PWM for DC motors
  int stepsPerRevolution = 500; // 28BYJ-48 Stepper motor typical value

  Stepper stepperMotor(stepsPerRevolution, STEPPER_IN1, STEPPER_IN3, STEPPER_IN2, STEPPER_IN4);
  Servo servoRotary, servoDigging, servoCovering;   // ESP32Servo objects



  void setup() {
    Serial.begin(115200);
    SerialBT.begin("Smart Truck"); // Bluetooth device name
    Serial.println("Bluetooth has started! Ready to receive commands.");

    // Initialize DC motor control pins
    pinMode(dcMotor1Pin1, OUTPUT);
    pinMode(dcMotor1Pin2, OUTPUT);
    pinMode(dcMotor2Pin1, OUTPUT);
    pinMode(dcMotor2Pin2, OUTPUT);
    
    // Initialize stepper motor pins
    // pinMode(stepPin, OUTPUT);
    // pinMode(dirPin, OUTPUT);

    // Initialize relay pins
    pinMode(rotoryMotorRelayPin, OUTPUT);
    pinMode(waterPumpRelayPin, OUTPUT);
    
    // Attach servo motors to their pins
    servoRotary.attach(servoRotaryPin);
    servoDigging.attach(servoDiggingPin);
    servoCovering.attach(servoCoveringPin);

    // Initial state: all motors off
    stopMotors();
    controlRotoryMotor(LOW);
    controlWaterPump(LOW);
  }

  void loop() {

    if(stepperOn)
    {
      controlStepper(stepperSpeed);
      //delay(50);
    }
    // Read incoming data from Bluetooth
    while (SerialBT.available()) {
      char incomingChar = SerialBT.read();
      if (incomingChar == '\n') {
        // Newline character indicates end of command
        command.trim(); // Remove leading/trailing whitespace
        Serial.print("Received command: ");
        Serial.println(command);

        // Process the command by calling the appropriate function
        if (command.equals("forward")) {
          forward();
        } 
        else if (command.equals("backward")) {
          backward();
        } 
        else if (command.equals("left")) {
          left();
        } 
        else if (command.equals("right")) {
          right();
        } 
        else if (command.equals("stop")) {
          stopMotors();
        } 
          else if (command.startsWith("changeSpeed:")) {
          motorSpeed = command.substring(12).toInt();
          Serial.println(motorSpeed);

        } 
        else if (command.startsWith("stepper_speed:")) 
        {
          
          int speed = command.substring(14).toInt();
          stepperOn = true;
          stepperSpeed = speed + 30;
          controlStepper(stepperSpeed);
        }
           else if (command.startsWith("stepper_off")) 
        {
          stepperOn = false;
        
        }
         else if (command.startsWith("servo_rotary:")) 
        {
          int angle = command.substring(13).toInt();
          controlServoRotary(angle);
        } 
        else if (command.startsWith("servo_digging:")) 
        {
          int angle = command.substring(14).toInt();
          controlServoDigging(angle);
        } 
        else if (command.startsWith("servo_covering:")) 
        {
          int angle = command.substring(15).toInt();
          controlServoCovering(angle);
        } 
        else if (command.equals("rotory_on")) 
        {
          controlRotoryMotor(HIGH);
        } 
        else if (command.equals("rotory_off")) 
        {
          controlRotoryMotor(LOW);
        } 
        else if (command.equals("pump_on")) 
        {
          controlWaterPump(HIGH);
        } 
        else if (command.equals("pump_off")) 
        {
          controlWaterPump(LOW);
        }
        
        // Clear the command string for the next command
        command = ""; 
      } else {
        command += incomingChar;
      }
    }
  }

  // --- Function Implementations ---

  // Controls DC motors for forward movement
  void forward() {


    digitalWrite(dcMotor1Pin1, LOW);
    digitalWrite(dcMotor1Pin2, HIGH);
    digitalWrite(dcMotor2Pin1, LOW);
    digitalWrite(dcMotor2Pin2, HIGH);

    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  // Controls DC motors for backward movement
  void backward() {

    digitalWrite(dcMotor1Pin1, HIGH);
    digitalWrite(dcMotor1Pin2, LOW);
    digitalWrite(dcMotor2Pin1, HIGH);
    digitalWrite(dcMotor2Pin2, LOW);

    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  // Controls DC motors to turn left
  void right() {
    digitalWrite(dcMotor1Pin1, HIGH);
    digitalWrite(dcMotor1Pin2, LOW);
    digitalWrite(dcMotor2Pin1, LOW);
    digitalWrite(dcMotor2Pin2, HIGH);

    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  // Controls DC motors to turn right
  void left() {
    digitalWrite(dcMotor1Pin1, LOW);
    digitalWrite(dcMotor1Pin2, HIGH);
    digitalWrite(dcMotor2Pin1, HIGH);
    digitalWrite(dcMotor2Pin2, LOW);

    analogWrite(ENA, motorSpeed);
    analogWrite(ENB, motorSpeed);
  }

  // Stops all DC motors
  void stopMotors() {
    digitalWrite(dcMotor1Pin1, LOW);
    digitalWrite(dcMotor1Pin2, LOW);
    digitalWrite(dcMotor2Pin1, LOW);
    digitalWrite(dcMotor2Pin2, LOW);
  }

  // Control stepper motor speed
  void controlStepper(int speed) {
      
    stepperMotor.setSpeed(speed); // RPM
    stepperMotor.step(-10);
  }

  // Control Servo Rotary position
  void controlServoRotary(int angle) {
    servoRotary.write(angle);
  }

  // Control Servo Digging position
  void controlServoDigging(int angle) {
    servoDigging.write(angle);
  }

  // Control Servo Covering position
  void controlServoCovering(int angle) {
    servoCovering.write(angle);
  }

  // Control relays
  void controlRotoryMotor(bool state) {
    digitalWrite(rotoryMotorRelayPin, state);
  }

  void controlWaterPump(bool state) {
    digitalWrite(waterPumpRelayPin, state);
  }

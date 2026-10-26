#include <AccelStepper.h>

// --- MKS BOARD PIN DEFINITIONS ---
#define X1_STEP_PIN 54
#define X1_DIR_PIN  55
#define Y_STEP_PIN  46
#define Y_DIR_PIN   48
#define X2_STEP_PIN 26
#define X2_DIR_PIN  28

#define Z_STEP_PIN     36
#define Z_DIR_PIN      34

#define Z_ENABLE_PIN   30
#define X_ENABLE_PIN      38  
#define Y_ENABLE_PIN      62  
#define E0_ENABLE_PIN     24  
#define ELECTROMAGNET_PIN 10  

AccelStepper stepperX1(AccelStepper::DRIVER, X1_STEP_PIN, X1_DIR_PIN);
AccelStepper stepperX2(AccelStepper::DRIVER, X2_STEP_PIN, X2_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STEP_PIN, Z_DIR_PIN);

const long JOG_STEPS = 400; // Steps per key press
const float MOTOR_SPEED = 15000.0;
const float MOTOR_ACCEL = 15000.0;
bool magnetState = false;
const int MAG = 10;

enum ZSequenceState {
  Z_IDLE,
  Z_MOVE_DOWN,
  Z_WAIT_DOWN,
  Z_MAGNET_ON_WAIT,
  Z_MOVE_UP,
  Z_WAIT_UP,
  Z_PLACE_DOWN,
  Z_PLACE_WAIT_DOWN,
  Z_PLACE_MAGNET_OFF_WAIT,
  Z_PLACE_UP,
  Z_PLACE_WAIT_UP
};

ZSequenceState zState = Z_IDLE;
unsigned long zTimer = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(X_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW); 
  pinMode(Y_ENABLE_PIN, OUTPUT);
  digitalWrite(Y_ENABLE_PIN, LOW); 
  pinMode(E0_ENABLE_PIN, OUTPUT);
  digitalWrite(E0_ENABLE_PIN, LOW); 
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(Z_ENABLE_PIN, LOW);

  pinMode(MAG, OUTPUT);
  digitalWrite(MAG, LOW);

  stepperX1.setMaxSpeed(MOTOR_SPEED);
  stepperX1.setAcceleration(MOTOR_ACCEL);
  stepperX2.setMaxSpeed(MOTOR_SPEED);
  stepperX2.setAcceleration(MOTOR_ACCEL);
  stepperY.setMaxSpeed(MOTOR_SPEED);
  stepperY.setAcceleration(MOTOR_ACCEL);
  stepperZ.setAcceleration(MOTOR_ACCEL);
  stepperZ.setMaxSpeed(MOTOR_SPEED);
  
  Serial.println("Ready for commands...");
}

void loop() {
  // Constantly poll steppers to keep them moving smoothly
  stepperX1.run();
  stepperX2.run();
  stepperY.run();
  stepperZ.run();

  // Check for commands from Python over Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    if (cmd == 'W') { // Up Arrow -> X Forward
      stepperX1.move(JOG_STEPS);
      stepperX2.move(JOG_STEPS);
      Serial.println("Moving X Forward");
    } 
    else if (cmd == 'S') { // Down Arrow -> X Backward
      stepperX1.move(-JOG_STEPS);
      stepperX2.move(-JOG_STEPS);
      Serial.println("Moving X Backward");
    }
    else if (cmd == 'A') { // W Key -> Y Forward
      stepperY.move(JOG_STEPS);
      Serial.println("Moving Y Forward");
    }
    else if (cmd == 'D') { // S Key -> Y Backward
      stepperY.move(-JOG_STEPS);
      Serial.println("Moving Y Backward");
    }
    else if (cmd == 'P' && zState == Z_IDLE) {
      Serial.println("Starting Z place sequence...");
      zState = Z_PLACE_DOWN;
    }
    else if (cmd == 'L' && zState == Z_IDLE) { 
      Serial.println("Starting Z pick sequence...");
      zState = Z_MOVE_DOWN;
    }
    else if (cmd == 'M') { // M Key -> Toggle Magnet
      magnetState = !magnetState;
      digitalWrite(MAG, magnetState ? HIGH : LOW);
      Serial.print("Magnet: ");
      Serial.println(magnetState ? "ON" : "OFF");
    }
  }
  switch (zState) {
    case Z_IDLE:
      break;

    case Z_MOVE_DOWN:
      stepperZ.move(-1200); // Adjust step distance to move down
      zState = Z_WAIT_DOWN;
      break;

    case Z_WAIT_DOWN:
      if (stepperZ.distanceToGo() == 0) {
        Serial.println("Z reached bottom. Turning Magnet ON.");
        digitalWrite(ELECTROMAGNET_PIN, HIGH);
        zTimer = millis();
        zState = Z_MAGNET_ON_WAIT;
      }
      break;

    case Z_MAGNET_ON_WAIT:
      if (millis() - zTimer >= 500) { // Wait 1 seconds with magnet on
        Serial.println("Moving Z back up.");
        stepperZ.move(1200); // Move back up by same step amount
        zState = Z_WAIT_UP;
      }
      break;

    case Z_WAIT_UP:
      if (stepperZ.distanceToGo() == 0) {
        Serial.println("Z reached top.");
        zState = Z_IDLE; // Sequence complete
      }
      break;

    case Z_PLACE_DOWN:
      stepperZ.move(-1200); // Move down
      zState = Z_PLACE_WAIT_DOWN;
      break;

    case Z_PLACE_WAIT_DOWN:
      if (stepperZ.distanceToGo() == 0) {
        Serial.println("Z reached bottom. Turning Magnet OFF.");
        digitalWrite(ELECTROMAGNET_PIN, LOW);
        zTimer = millis();
        zState = Z_PLACE_MAGNET_OFF_WAIT;
      }
      break;

    case Z_PLACE_MAGNET_OFF_WAIT:
      if (millis() - zTimer >= 500) { // Wait 2 seconds with magnet off
        Serial.println("Moving Z back up.");
        stepperZ.move(1200); // Move back up
        zState = Z_PLACE_WAIT_UP;
      }
      break;

    case Z_PLACE_WAIT_UP:
      if (stepperZ.distanceToGo() == 0) {
        Serial.println("Place complete. Magnet remains OFF.");
        zState = Z_IDLE;
      }
      break;
  }
  }



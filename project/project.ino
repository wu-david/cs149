#include <Servo.h>
#include <IRremote.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>

/* pin definitions
 * 3 - IR TX
 * 5 - BT RX-1
 * 6 - BT TX-0
 * 9 - Servo
 * 10 - Gun switch
 * 11 - IR RX
 * 13 - LED
 */
#define IRTXPIN 3
#define BTRXPIN 5
#define BTTXPIN 6
#define SERVOPIN 9
#define GUNSWITCHPIN 10
#define IRRXPIN 11
#define LEDPIN 13

/* states for the lock */
enum LockState {
  locked,
  validateUnlock,
  validateLock,
  unlocking,
  unlocked,
  locking
};

/* states for the gun */
enum GunState {
  unarmed,
  armed,
  firing,
  validate
};

/* declarations */
Servo lockServo;
String dataFromBT;
IRsend irsend;
unsigned long unlockTime;
unsigned long armTime;
unsigned long firingTime;
double accelMag;

/* assignments */
boolean validated = false;
LockState lockState = locked;
GunState gunState = unarmed;
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
String msg = "";
const unsigned long LOCK_TO = 10000;
const unsigned long GUN_TO = 5000;
const unsigned long FIRING_TO = 5000;
const int SERVO_LOCK = 0;
const int SERVO_UNLOCK = 90;
const int IR_TRIPPED = 1;
const int IR_NOT_TRIPPED = 0;
const int PEWPEW = 255;
const int NOTPEWPEW = 0;
const int ACCEL_THRESHOLD = 25;

void setup() {
  /* servo setup */
  lockServo.attach(SERVOPIN);
  lockServo.write(SERVO_LOCK);
  
  /* serial monitor setup */
  //Serial.begin(9600);
  //Serial.println("Serial begins... ");

  /* bluetooth setup */
  Serial.begin(115200);
  dataFromBT = "";
  
  /* ir setup */
  pinMode(IRRXPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  irsend.enableIROut(38);
  irsend.mark(0);
  
  /* accelerometer setup */
  accel.begin();
  if(!accel.begin()) {
    Serial.println(F("LSM303 not detected"));
    while(1);
  }
  
  /* gun switch setup */
  pinMode(GUNSWITCHPIN, OUTPUT);
  analogWrite(GUNSWITCHPIN, NOTPEWPEW);
  
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);
  accelMag = calculate(event.acceleration.x, event.acceleration.y, event.acceleration.z);
  
  /* lock transitions */
  if (lockState == locked && Serial.available()) {
    validated = false;
    lockState = validateUnlock;
  } else if (lockState == unlocking && lockServo.read() == SERVO_UNLOCK) {
    lockState = unlocked;
    unlockTime = millis();
  } else if (lockState == unlocked && millis() - unlockTime > LOCK_TO) {
    lockState = locking;
  } else if (lockState == unlocked && Serial.available()) {
    validated = false;
    lockState = validateLock;
  } else if (lockState == locking && lockServo.read() == SERVO_LOCK) {
    lockState = locked;
  }
  
  /* gun transitions */
  if (gunState == unarmed && digitalRead(IRRXPIN) == IR_TRIPPED) {
    gunState = armed;
    armTime = millis();
  } else if (gunState == armed && digitalRead(IRRXPIN) == IR_NOT_TRIPPED && millis() - armTime > GUN_TO) {
    gunState = unarmed;
  } else if (gunState == armed && accelMag > ACCEL_THRESHOLD) {
    gunState = firing;
    firingTime = millis();
  } else if (gunState == firing && millis() - firingTime > FIRING_TO) {
    gunState = armed;
  }
  
  /* lock actions */
  switch (lockState) {
    case locked:
      break;
    case validateUnlock:
      while (Serial.available()) {
        char received = Serial.read();
        if (received == ',') {
          validateKey(dataFromBT, "locked") ? lockState = unlocking : lockState = locked;
          dataFromBT = "";
          break;
        }
        dataFromBT.concat(String(received)); 
      }
      break;
    case validateLock:
      while (Serial.available()) {
        char received = Serial.read();
        if (received == ',') {
          validateKey(dataFromBT, "unlocked") ? lockState = locking : lockState = unlocked;
          dataFromBT = "";
          break;
        }
        dataFromBT.concat(String(received));
      }
      break;
    case unlocking:
      lockServo.write(SERVO_UNLOCK);
      break;
    case unlocked:
      break;
    case locking:
      lockServo.write(SERVO_LOCK);
      break;
    default:
      break;
  }
  
  /* gun actions */
  switch (gunState) {
    case unarmed:
      digitalWrite(LEDPIN, HIGH);
      analogWrite(GUNSWITCHPIN, NOTPEWPEW);
      break;
    case armed:
      digitalWrite(LEDPIN, LOW);
      analogWrite(GUNSWITCHPIN, NOTPEWPEW);
      break;
    case firing:
      digitalWrite(LEDPIN, LOW);
      analogWrite(GUNSWITCHPIN, PEWPEW);
      break;
    case validate:
      break;
  }
  delay(50);
}

double calculate(float x, float y, float z) {
  return sqrt((x*x)+(y*y)+(z*z));
}

boolean validateKey(String msg, String type) {
  if (type == "unlocked") {
    //TODO: Change to RSA
    if (msg == "lock 1234321") return true;
  }
  if (type == "locked") {
    //TODO: Change to RSA
    if (msg == "unlock 1234321") return true;
  }
  return false;
}

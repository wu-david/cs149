#include <Servo.h>

int servoPin = 9;

Servo myservo;

int val;

void setup() {
  myservo.attach(servoPin);
  myservo.write(90);  // set servo to mid-point
  Serial.begin(9600);
  Serial.println("ready");
}

void loop() {
  if (Serial) {
    val = Serial.read();
    if (val == '2') {
      myservo.write(90);
      delay(20);
    }
    if (val == '1') {
      myservo.write(0);
      delay(20);
    }
  }
}



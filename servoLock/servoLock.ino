#include <SoftwareSerial.h>
#include <Servo.h>
#include <IRremote.h>

/* pin definitions
 * 3 - IR TX
 * 5 - BT RX-1
 * 6 - BT TX-0
 * 9 - Servo
 * 11 - IR RX
 * 13 - LED
 */
#define IRTXPIN 3
#define BTRXPIN 5
#define BTTXPIN 6
#define SERVOPIN 9
#define IRRXPIN 11
#define LEDPIN 13


/* declarations */
Servo lockServo;
SoftwareSerial bluetooth(BTTXPIN, BTRXPIN);
String dataFromBT;
IRsend irsend;
 
void setup() {
  /* servo setup */
  lockServo.attach(SERVOPIN);
  lockServo.write(90);
  
  /* serial monitor setup */
  Serial.begin(9600);
  Serial.println("Serial begins... ");

  /* bluetooth setup */
  bluetooth.begin(115200);
  dataFromBT = "";
  
  /* ir setup */
  pinMode(IRRXPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  irsend.enableIROut(38);
  irsend.mark(0);
}
 
void loop() {
  digitalWrite(LEDPIN, !digitalRead(IRRXPIN));
  while (bluetooth.available()) {
    char received = bluetooth.read();
    Serial.println("Got int: " + String((byte)received));
    dataFromBT.concat(String(received));
    
    if (received == ',') {
      Serial.println("Arduino Received: " +  dataFromBT);  
      dataFromBT = "";
    } else {
      if (dataFromBT == "off") {
        Serial.println("turning to 90");
        lockServo.write(90); // turn to 90
        delay(20);
        dataFromBT = "";
      } else if (dataFromBT == "on") {
        Serial.println("turning to 0");
        lockServo.write(0); // turn to 0
        delay(20);
        dataFromBT = "";
      }
    }
  }  
}

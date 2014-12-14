#include <SoftwareSerial.h>
#include <Servo.h>

/* pin definitions
 * 5 - BT RX-1
 * 6 - BT TX-0
 * 9 - Servo
 */
int rx = 5;
int tx = 6;
int servoPin = 9;

/* declarations */
Servo lockServo;
SoftwareSerial bluetooth(tx, rx);
String dataFromBT;
 
void setup() {
  /* servo setup */
  lockServo.attach(servoPin);
  lockServo.write(90);
  
  /* serial monitor setup */
  Serial.begin(9600);
  Serial.println("Serial begins... ");

  /* bluetooth setup */
  bluetooth.begin(115200);
  dataFromBT = "";
}
 
void loop() {
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

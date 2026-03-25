#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h> 

const char* ssid = "iot";
const char* password = "4257530993";

WiFiUDP udp;
const int localPort = 4210;

const int ledPin = 8;
const int servoPin = 4;

Servo steeringServo;

void setup() {

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);   

  steeringServo.setPeriodHertz(50);      
  steeringServo.attach(servoPin, 500, 2500); 

  WiFi.begin(ssid, password);
  udp.begin(localPort);
}

void loop() {
  
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    return;
  }

  int packetSize = udp.parsePacket();

  if (packetSize == 2) {

    byte buffer[2];
    udp.read(buffer, 2);

    uint8_t throttle = buffer[0];
    int8_t steering = (int8_t)buffer[1];


    if (throttle == 255) {
      digitalWrite(ledPin, LOW);   
    } else {
      digitalWrite(ledPin, HIGH);
    }



    int angle = map(steering, -100, 100, 0, 180);
    angle = constrain(angle, 0, 180);

    steeringServo.write(angle);
  }
}
#include <WiFi.h>
#include <WiFiUdp.h>

// --- WIFI SETUP ---
const char* ssid = "auton";
const char* password = "12345678";

WiFiUDP udp;
const int port = 4210;

int8_t buffer[2];

// --- LED SETUP ---
const int LED_PIN = 8;
unsigned long lastBlink = 0;
bool ledState = false;

// --- LEFT MOTOR PINS ---
const int LEFT_IN1 = 2;
const int LEFT_IN2 = 3;

// --- RIGHT MOTOR PINS ---
const int RIGHT_IN1 = 4;
const int RIGHT_IN2 = 5;

unsigned long lastPacketTime = 0;
const unsigned long FAILSAFE_TIMEOUT = 500;

void setOneMotor(int in1, int in2, int command) {
  if (command > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } 
  else if (command < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } 
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void stopMotors() {
  setOneMotor(LEFT_IN1, LEFT_IN2, 0);
  setOneMotor(RIGHT_IN1, RIGHT_IN2, 0);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  delay(1000);
  
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(port);
  Serial.println("UDP listening...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  stopMotors();
}

void loop() {
  int clients = WiFi.softAPgetStationNum();

  if (clients == 0) {
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  int packetSize = udp.parsePacket();

  if (packetSize == 2) {
    udp.read((uint8_t*)buffer, 2);

    int8_t leftCommand = buffer[0];
    int8_t rightCommand = buffer[1];

    if (abs(leftCommand) < 15) leftCommand = 0;
    if (abs(rightCommand) < 15) rightCommand = 0;

    setOneMotor(LEFT_IN1, LEFT_IN2, leftCommand);
    setOneMotor(RIGHT_IN1, RIGHT_IN2, rightCommand);

    lastPacketTime = millis();

    Serial.print("Left: ");
    Serial.print(leftCommand);
    Serial.print(" | Right: ");
    Serial.println(rightCommand);
  }

  if (millis() - lastPacketTime > FAILSAFE_TIMEOUT) {
    stopMotors();
  }
}
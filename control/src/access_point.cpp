#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// --- WIFI SETUP ---
const char* ssid = "auton";
const char* password = "12345678";

WiFiUDP udp;
const int port = 4210;

uint8_t buffer[2];

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

void setOneMotor(int in1, int in2, int dir) {
  if (dir > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } 
  else if (dir < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } 
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void setDrive(float throttle, float steer) {
  throttle = constrain(throttle, -1.0, 1.0);
  steer = constrain(steer, -1.0, 1.0);

  float deadzone = 0.15;

  if (fabs(throttle) < deadzone && fabs(steer) < deadzone) {
    setOneMotor(LEFT_IN1, LEFT_IN2, 0);
    setOneMotor(RIGHT_IN1, RIGHT_IN2, 0);
    return;
  }

  int leftDir = 0;
  int rightDir = 0;

  if (fabs(throttle) >= deadzone) {
    if (throttle > 0) {
      leftDir = 1;
      rightDir = 1;
    } else {
      leftDir = -1;
      rightDir = -1;
    }

    if (steer > deadzone) {
      rightDir = 0;
    } 
    else if (steer < -deadzone) {
      leftDir = 0;
    }
  } 
  else {
    if (steer > deadzone) {
      leftDir = 1;
      rightDir = -1;
    } 
    else if (steer < -deadzone) {
      leftDir = -1;
      rightDir = 1;
    }
  }

  setOneMotor(LEFT_IN1, LEFT_IN2, leftDir);
  setOneMotor(RIGHT_IN1, RIGHT_IN2, rightDir);

  Serial.print("Left Dir: ");
  Serial.print(leftDir);
  Serial.print(" | Right Dir: ");
  Serial.println(rightDir);
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

  setOneMotor(LEFT_IN1, LEFT_IN2, 0);
  setOneMotor(RIGHT_IN1, RIGHT_IN2, 0);
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
    udp.read(buffer, 2);

    uint8_t throttle_byte = buffer[0];
    int8_t steer_byte = buffer[1];

    float throttle = ((float)throttle_byte / 255.0) * 2.0 - 1.0;
    float steer = steer_byte / 100.0;

    setDrive(throttle, steer);

    Serial.print("Throttle: ");
    Serial.print(throttle, 2);
    Serial.print(" | Steer: ");
    Serial.println(steer, 2);
  }
}
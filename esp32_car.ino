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
const int LEFT_EN  = 4;

// --- RIGHT MOTOR PINS ---
const int RIGHT_IN1 = 5;
const int RIGHT_IN2 = 6;
const int RIGHT_EN  = 7;

// --- PWM ---
const int LEFT_PWM_CHANNEL  = 0;
const int RIGHT_PWM_CHANNEL = 1;

const int PWM_FREQ = 1000;
const int PWM_RES  = 8;

void setOneMotor(int in1, int in2, int pwmChannel, int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(pwmChannel, speed);
  } 
  else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(pwmChannel, -speed);
  } 
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(pwmChannel, 0);
  }
}

void setDrive(float throttle, float steer) {
  throttle = constrain(throttle, -1.0, 1.0);
  steer = constrain(steer, -1.0, 1.0);

  float deadzone = 0.05;

  if (fabs(throttle) < deadzone) {
    throttle = 0;
  }

  if (fabs(steer) < deadzone) {
    steer = 0;
  }

  int drive = throttle * 255;
  int turn = steer * 255;

  int leftSpeed = drive + turn;
  int rightSpeed = drive - turn;

  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  setOneMotor(LEFT_IN1, LEFT_IN2, LEFT_PWM_CHANNEL, leftSpeed);
  setOneMotor(RIGHT_IN1, RIGHT_IN2, RIGHT_PWM_CHANNEL, rightSpeed);

  Serial.print("Left PWM: ");
  Serial.print(leftSpeed);
  Serial.print(" | Right PWM: ");
  Serial.println(rightSpeed);
}

void setup() {
  Serial.begin(115200);

  // --- WIFI AP ---
  WiFi.softAP(ssid, password);

  delay(1000);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(port);
  Serial.println("UDP listening...");

  // --- LED ---
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // --- MOTOR PINS ---
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  // --- PWM SETUP ---
  ledcSetup(LEFT_PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcSetup(RIGHT_PWM_CHANNEL, PWM_FREQ, PWM_RES);

  ledcAttachPin(LEFT_EN, LEFT_PWM_CHANNEL);
  ledcAttachPin(RIGHT_EN, RIGHT_PWM_CHANNEL);

  setOneMotor(LEFT_IN1, LEFT_IN2, LEFT_PWM_CHANNEL, 0);
  setOneMotor(RIGHT_IN1, RIGHT_IN2, RIGHT_PWM_CHANNEL, 0);
}

void loop() {
  // --- CLIENT STATUS LED ---
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

  // --- UDP RECEIVE ---
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
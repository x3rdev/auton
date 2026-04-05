#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// ─── AP Config ───────────────────────────────────────────────
const char* ssid     = "ESP32-Car";
const char* password = "12345678";

// ─── UDP ─────────────────────────────────────────────────────
WiFiUDP udp;
const int localPort = 4210;

// ─── Pins ────────────────────────────────────────────────────
const int ledPin      = 8;
const int servoPin    = 4;   // Steering servo
const int motorFwdPin = 18;  // Motor forward  (to L298N IN1 or ESC)
const int motorRevPin = 19;  // Motor reverse   (to L298N IN2)
const int motorPWMPin = 21;  // PWM speed pin   (to L298N ENA)

// ─── PWM channel for motor (ESP32 LEDC) ──────────────────────
const int motorChannel  = 0;
const int motorFreq     = 1000;  // 1 kHz
const int motorResolution = 8;   // 8-bit → 0-255

Servo steeringServo;

// ─── Helpers ─────────────────────────────────────────────────
void setMotor(uint8_t throttle) {
  // throttle: 0-255
  // 0        = full reverse
  // 127      = stop
  // 255      = full forward

  if (throttle > 137) {
    // Forward
    digitalWrite(motorFwdPin, HIGH);
    digitalWrite(motorRevPin, LOW);
    int speed = map(throttle, 138, 255, 0, 255);
    ledcWrite(motorChannel, speed);

  } else if (throttle < 117) {
    // Reverse
    digitalWrite(motorFwdPin, LOW);
    digitalWrite(motorRevPin, HIGH);
    int speed = map(throttle, 116, 0, 0, 255);
    ledcWrite(motorChannel, speed);

  } else {
    // Dead zone → stop
    digitalWrite(motorFwdPin, LOW);
    digitalWrite(motorRevPin, LOW);
    ledcWrite(motorChannel, 0);
  }
}

// ─── Setup ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // Motor pins
  pinMode(motorFwdPin, OUTPUT);
  pinMode(motorRevPin, OUTPUT);
  // ledcSetup(motorChannel, motorFreq, motorResolution); 
  ledcAttach(motorPWMPin, motorChannel, motorResolution);

  // Steering servo
  steeringServo.setPeriodHertz(50);
  steeringServo.attach(servoPin, 500, 2500);
  steeringServo.write(90);  // Centre on boot

  // Start AP
  WiFi.softAP(ssid, password);
  Serial.println("AP started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());  // Always 192.168.4.1

  udp.begin(localPort);
  Serial.println("UDP ready on port " + String(localPort));
}

// ─── Loop ────────────────────────────────────────────────────
void loop() {
  int packetSize = udp.parsePacket();

  if (packetSize >= 2) {
    byte buffer[2];
    udp.read(buffer, 2);

    uint8_t throttle = buffer[0];   // 0=full reverse, 127=stop, 255=full forward
    int8_t  steering = (int8_t)buffer[1];  // -100 to 100

    Serial.print("Throttle: ");
    Serial.print(throttle);
    Serial.print(" | Steering: ");
    Serial.println(steering);

    // Motor
    setMotor(throttle);

    // LED: blink when moving forward
    digitalWrite(ledPin, throttle > 137 ? LOW : HIGH);

    // Steering servo
    int angle = map(steering, -100, 100, 0, 180);
    angle = constrain(angle, 0, 180);
    steeringServo.write(angle);
  }
}

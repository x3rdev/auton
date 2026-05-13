#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>
#include <ESP32Servo.h>

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

// --- MOTOR PINS ---
const int IN1 = 3;
const int IN2 = 4;
const int ENA = 5;

// --- PWM (MOTOR) ---
const int PWM_CHANNEL_MOTOR = 2;   
const int PWM_FREQ_MOTOR = 1000;
const int PWM_RES_MOTOR = 8;

// --- SERVO ---
Servo steeringServo;
const int SERVO_PIN = 7;

// --- SERVO FUNCTION ---
void setServo(float steer) {
  if (steer > 1) steer = 1;
  if (steer < -1) steer = -1;

  int angle = (int)((steer + 1.0) * 90.0);  // -1→1 → 0→180
  steeringServo.write(angle);
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

  // --- MOTOR ---
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ESP32PWM::allocateTimer(0); // motor
  ESP32PWM::allocateTimer(1); // servo

  ledcSetup(PWM_CHANNEL_MOTOR, PWM_FREQ_MOTOR, PWM_RES_MOTOR);
  ledcAttachPin(ENA, PWM_CHANNEL_MOTOR);

  // --- SERVO ---
  steeringServo.setPeriodHertz(50);
  steeringServo.attach(SERVO_PIN, 500, 2500);

  // Center servo
  setServo(0);

  // --- TEST SWEEP (remove later if you want) ---
  delay(500);
  steeringServo.write(0);
  delay(800);
  steeringServo.write(180);
  delay(800);
  steeringServo.write(90);
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
    int8_t steer_byte     = buffer[1];

    float throttle = ((float)throttle_byte / 255.0) * 2.0 - 1.0;
    float steer = steer_byte / 100.0;

    // --- MOTOR CONTROL ---
    float deadzone = 0.05;

    if (fabs(throttle) < deadzone) {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      ledcWrite(PWM_CHANNEL_MOTOR, 0);
    }
    else if (throttle > 0) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      ledcWrite(PWM_CHANNEL_MOTOR, (int)(throttle * 255));
    }
    else {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      ledcWrite(PWM_CHANNEL_MOTOR, (int)(-throttle * 255));
    }

    // --- SERVO CONTROL ---
    setServo(steer);

    // --- DEBUG ---
    Serial.print("Throttle: ");
    Serial.print(throttle, 2);
    Serial.print(" | PWM: ");
    Serial.print((int)(fabs(throttle) * 255));
    Serial.print(" | Steer: ");
    Serial.println(steer, 2);
  }
}
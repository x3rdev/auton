#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// --- WIFI SETUP ---
const char* ssid = "auton";
const char* password = "12345678";

WiFiUDP udp;
const int port = 4210;

uint8_t buffer[2];  // exactly 2 bytes

// --- LED SETUP ---
const int LED_PIN = 8;
unsigned long lastBlink = 0;
bool ledState = false;

// --- MOTOR PINS (ESP32-C3 SUPER MINI) ---
const int IN1 = 3;
const int IN2 = 4;
const int ENA = 5;

// --- PWM SETUP ---
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 1000;
const int PWM_RES = 8; // 0–255

void setup() {
  Serial.begin(115200);

  // Start Access Point
  WiFi.softAP(ssid, password);

  delay(1000);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(port);
  Serial.println("UDP listening...");

  // LED setup (inverted)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // OFF

  // MOTOR setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA, PWM_CHANNEL);
}

void loop() {
  // --- CLIENT CHECK ---
  int clients = WiFi.softAPgetStationNum();

  if (clients == 0) {
    // Blink when no clients
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    }
  } else {
    // Solid ON when connected
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
      // STOP
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      ledcWrite(PWM_CHANNEL, 0);
    }
    else if (throttle > 0) {
      // FORWARD
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);

      int speed = (int)(throttle * 255);
      ledcWrite(PWM_CHANNEL, speed);
    }
    else {
      // REVERSE
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);

      int speed = (int)(-throttle * 255);
      ledcWrite(PWM_CHANNEL, speed);
    }

    // --- DEBUG PRINT ---
    Serial.print("RAW: ");
    Serial.print(throttle_byte);
    Serial.print(" , ");
    Serial.print(steer_byte);

    Serial.print(" | Throttle: ");
    Serial.print(throttle, 3);

    Serial.print(" | Speed PWM: ");
    Serial.println((int)(fabs(throttle) * 255));
  }
}
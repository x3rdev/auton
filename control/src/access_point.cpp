#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "auton";
const char* password = "12345678";

WiFiUDP udp;
const int port = 4210;

uint8_t buffer[2];  // exactly 2 bytes

// --- LED SETUP ---
const int LED_PIN = 8;
unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  // Start Access Point
  WiFi.softAP(ssid, password);

  delay(1000);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(port);
  Serial.println("UDP listening...");

  // LED setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // OFF (inverted)
}

void loop() {
  // --- CLIENT CHECK ---
  int clients = WiFi.softAPgetStationNum();

  if (clients == 0) {
    // Blink when no clients connected
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      ledState = !ledState;

      // Inverted logic
      digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    }
  } else {
    // Solid ON when connected
    digitalWrite(LED_PIN, LOW);  // ON (inverted)
  }

  // --- UDP RECEIVE ---
  int packetSize = udp.parsePacket();

  if (packetSize == 2) {
    udp.read(buffer, 2);

    // --- RAW BYTES ---
    uint8_t throttle_byte = buffer[0];   // 0–255
    int8_t steer_byte     = buffer[1];   // -128–127

    // --- DECODE VALUES ---
    float throttle = ((float)throttle_byte / 255.0) * 2.0 - 1.0;
    float steer = steer_byte / 100.0;

    // --- PRINT ---
    Serial.print("RAW: ");
    Serial.print(throttle_byte);
    Serial.print(" , ");
    Serial.print(steer_byte);

    Serial.print(" | Decoded → Throttle: ");
    Serial.print(throttle, 3);

    Serial.print(" | Steer: ");
    Serial.println(steer, 3);
  }
}
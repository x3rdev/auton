#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "auton";
const char* password = "12345678";

WiFiUDP udp;
const int port = 4210;

uint8_t buffer[2];  // exactly 2 bytes

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(port);
  Serial.println("UDP listening...");
}

void loop() {
  int packetSize = udp.parsePacket();

  if (packetSize == 2) {
    udp.read(buffer, 2);

    // --- RAW BYTES ---
    uint8_t throttle_byte = buffer[0];   // 0–255
    int8_t steer_byte     = buffer[1];   // -128–127

    // --- DECODE VALUES ---
    // Map throttle back to -1.0 → 1.0
    float throttle = ((float)throttle_byte / 255.0) * 2.0 - 1.0;

    // Steering already roughly -100 → 100
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
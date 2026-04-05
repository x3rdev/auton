#include <WiFi.h>

const char* ssid = "auton";
const char* password = "12345678";

#define LED_PIN 8   // change to 2 if needed

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // start OFF

  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  int clients = WiFi.softAPgetStationNum();

  if (clients > 0) {
    digitalWrite(LED_PIN, LOW);  // LED ON
    // Serial.println("User Connected");
  } else {
    digitalWrite(LED_PIN, HIGH);   // LED OFF 
    // Serial.println("No Users");
  }

  delay(500);  // small debounce
}
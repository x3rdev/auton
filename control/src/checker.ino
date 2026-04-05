void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ALIVE");
}

void loop() {
  Serial.println("running...");
  delay(1000);
}
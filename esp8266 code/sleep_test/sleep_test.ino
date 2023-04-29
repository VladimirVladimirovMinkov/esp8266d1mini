void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
}

void loop() {
  delay(1);
  ESP.deepSleep(5e6);
  ESP.esp_light_sleep_start(5e6);
}

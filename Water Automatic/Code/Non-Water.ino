void setup() {
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(9) == 1) {
    Serial.println("Water Detect"); // Motor ไม่ทำงาน
  } else {
    Serial.println("No Water"); // Motor ทำงาน
  }
  delay(1000);
}

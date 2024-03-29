// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib

#include "Wire.h"
#include "SPI.h"  // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"
#include "Servo.h"

RTC_DS3231 RTC;
Servo myservo;

// stop the food from flowing
void feederClose() {
  myservo.write(145);
}

// release a ration of food
void feederOpen() {
  myservo.write(180);
}

void setup () {
  
  myservo.attach(9);
  
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  DateTime now = RTC.now();
// ตั้งเวลา ในตัวอย่างนี้ เซตค่าเป็นเวลา 23:09 ถ้าถึงเวลานี้จะให้ทำงานที่ฟังก์ชัน 
  RTC.setAlarm1Simple(16,16);
   if (RTC.checkIfAlarm(1)) {
   Serial.println("Alarm Triggered");
   }

  RTC.turnOnAlarm(1);

  if (RTC.checkAlarmEnabled(1)) {
    Serial.println("Alarm Enabled");
  }

}

void loop () {
  
  DateTime now = RTC.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  if (RTC.checkIfAlarm(1)) {
    Serial.println("Feed Food");
    feederOpen();
    delay(5000);
    feederClose();
  }
  else{
    myservo.write(145);
  }
  delay(1000);
}

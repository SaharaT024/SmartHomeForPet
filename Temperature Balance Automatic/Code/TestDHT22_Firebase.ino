#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <L298N.h>

// Config Firebase
#define FIREBASE_HOST "testdht22-e02fb.firebaseio.com"
#define FIREBASE_AUTH "HO1M8U4d8Ojdr9uRqDByRVfcxA3FiHgf9debbXNJ"

// Config connect WiFi
#define WIFI_SSID "game"
#define WIFI_PASSWORD "123456789"

// Config DHT
#define DHTPIN D4
#define DHTTYPE DHT22

// Config Fan
#define EN D0
#define IN1 D6
#define IN2 D7

// Config time
int timezone = 7;

char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";

int dst = 0;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
L298N motor(EN, IN1, IN2);

void setup() {

  pinMode(DHTPIN, INPUT);

  Serial.begin(9600);

  Wire.begin(D2,D1);
  lcd.begin();

  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("Waiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Now: " + NowString());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  dht.begin();
  motor.setSpeed(80); // an integer between 0 and 255
}

void loop() {
   delay(100);
  // Read temp & Humidity for DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  motor.forward();
  String fs = "Prepare Fan";
  fs = FanStatus(t,fs);

  delay(100);
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DHT Failed");
  }
  else{
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(FanStatus(t,fs));
  Serial.println();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp : ");
  lcd.print(t);
  lcd.print("*C");
  lcd.setCursor(0,1);
  lcd.print("Humi : ");
  lcd.print(h);
  lcd.print("%");

  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Status: ");
  lcd.setCursor(0,1);
  lcd.print(fs);

  delay(100);
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = t;
  root["humidity"] = h;
  root["time"] = NowString();
  root["status"] = FanStatus(t,fs);

  delay(2500);

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("WiFi Disconnect");
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("WiFi Failed");
     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
     Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
     delay(500);    
  }
  else{
    // append a new value to /logDHT
    String name = Firebase.push("logDHT", root);
    delay(100);
    // handle error
    if(Firebase.success())
    {
      Serial.print("pushed: /logDHT/");
      Serial.println(name);
    }
    if (Firebase.failed()) {
      /*Serial.print("pushing /logDHT failed:");
      Serial.println(Firebase.error());
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Firebase Failed");*/
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      delay(100);
      return ;
    }
  }
  delay(2500);
  }
}

String NowString() {
  time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);

  String tmpNow = "";
  tmpNow += String(newtime->tm_hour);
  tmpNow += ":";
  tmpNow += String(newtime->tm_min);
  tmpNow += ":";
  tmpNow += String(newtime->tm_sec);
  return tmpNow;
}

String FanStatus(float t,String fs){
  if(t >= 30){
    motor.setSpeed(255);
    fs = "Enable 2";
  }
  else if(t >= 25 && t < 30){
    motor.setSpeed(128);
    fs = "Enable 1";
  }
  else{
    motor.stop();
    fs = "Disable";
  }
  return fs;
}


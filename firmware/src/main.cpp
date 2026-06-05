#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TFT_eSPI.h>

//Pin Definitions
#define BUZZER_PIN 25
#define VIBRATION_PIN 26
#define BUTTON_PIN 33
#define ENCODER_CLK 34
#define ENCODER_DT 35

//ALARM SETTINGS
int alarmHour = 8;
int alarmMinute = 0;
bool alarmActive = false;
bool alarmRinging = false;

//Snooze Settings
int snoozeCount = 0;
const int MAX_SNOOZE = 4;
const int SNOOZE_DURATION = 180000; // 3 minutes
unsigned long snoozeStartTime = 0;
unsigned long buttonPressStart = 0;
bool buttonHeld = false;

//Shake Detection
MPU6050 mpu;
TFT_eSPI tft = TFT_eSPI ();

void startAlarm() {
  alarmRinging = true;
  digitalWrite(VIBRATION_PIN, HIGH);
  tone(BUZZER_PIN, 440);
  Serial.println("ALARM RINGING!");
}

void stopAlarm() {
  alarmRinging = false;
  snoozeCount = 0;
  digitalWrite(VIBRATION_PIN, LOW);
  noTone(BUZZER_PIN);
  Serial.println("Alarm dismissed!");
}

void snoozeAlarm() {
  if (snoozeCount >= MAX_SNOOZE) {
    //Miku gives up after 4 snoozes
    stopAlarm ();
    Serial.println("Miku is upset. Alarm stopped");
    return;
  }
  snoozeCount++;
  alarmRinging = false;
  digitalWrite(VIBRATION_PIN, LOW);
  noTone(BUZZER_PIN);
  snoozeStartTime = millis() ;
  Serial.print("Snoozed! Count:");
  Serial.println(snoozeCount);
}

bool detectShake() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float magnitude = sqrt(ax*ax + ay*ay + az*az);
  return magnitude>20000; //shake threshold
}

void setup () {
  Serial.begin(115200);


//Pin setup
pinMode(BUZZER_PIN, OUTPUT);
pinMode(VIBRATION_PIN, OUTPUT);
pinMode(BUTTON_PIN, INPUT_PULLUP);
pinMode(ENCODER_CLK, INPUT);
pinMode(ENCODER_DT, INPUT);

//MPU6050 Setup
Wire.begin();
mpu.initialize();

//TFT Setup
tft.init() ;
tft.setRotation(1) ;
tft.fillScreen(TFT_BLACK) ;
tft.setTextColor(TFT_CYAN) ;
tft.setTextSize(2) ;
tft.println("Kusuri Dhappa!");
tft.println("Wake up!");

Serial.println("Kusuri Dhappa Ready!");
}

void loop() {
  //Check snooze timer
  if (!alarmRinging && snoozeCount>0){
    if (millis() -snoozeStartTime >= SNOOZE_DURATION) {
      //increase intensity each snooze startAlarm();
      tone(BUZZER_PIN, 440 + (snoozeCount*100));
    }
  }

  if (alarmRinging){
    //Check shake to dismiss
    if (detectShake()) {
      stopAlarm() ;
      return;
    }

    //Check button hold 20 seconds
    if (digitalRead(BUTTON_PIN)== LOW){
      if (!buttonHeld){
        buttonPressStart = millis();
        buttonHeld = true;
      }
      if (millis() - buttonPressStart >= 20000){
        stopAlarm() ;
        buttonHeld = false ;
      }
    } else {
      buttonHeld = false ;
    }

    //Auto snooze after 45 seconds 
    if (millis() - snoozeStartTime >= 45000 
    &&snoozeCount == 0) {
      snoozeAlarm();
    }
  }

  delay(100);
}


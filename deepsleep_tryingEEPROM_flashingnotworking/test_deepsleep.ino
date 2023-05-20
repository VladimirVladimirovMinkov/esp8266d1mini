#include <Arduino.h>
#include <EEPROM.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"

#include "viola.h"

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;

bool waitSeconds(int sec) {
  digitalWrite(D3, LOW);
  ESP.deepSleep(sec * 1e6);
  return false;
}

int beeps(int n){
  unsigned long StartTime=0;
  int t = 0;
  while (n>0){
    unsigned long CT = millis()-StartTime;
    if (CT%1000<500){
      digitalWrite(D3, LOW);
    } else if (CT%1000>500) {
      digitalWrite(D3, HIGH);
      if (t<CT%1000) {t=CT%1000;}
      else if (t>CT%1000) {t=0;n--;}
    }
  }
  return 0;
}

void setup()
{
  EEPROM.begin(1);
  Serial.begin(115200);
  delay(100);

  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT); // LED outpput

  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
  Serial.println("run");
  
  int count;
  count=EEPROM.read(0);
  
  count++;
  if (count > 5) {count = 1;}
  EEPROM.write(0, count);
  delay(10);
}

void loop()
{
  int count;
  count = EEPROM.read(0);
  
  if (digitalRead(4) == LOW) {
    if (wav->loop()) {
      if (digitalRead(4) == HIGH) {
        if (!wav->loop()) wav->stop();
      }
    } else {
      Serial.println("ran");
      Serial.println(count);
      waitSeconds(1*count);
    }
  } else {
    Serial.println("Trap has been triggered");
    waitSeconds(3*count);
  }
}

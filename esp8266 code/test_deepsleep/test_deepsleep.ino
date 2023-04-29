#include <Arduino.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"

#include "viola.h"

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;

void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(4, INPUT);

  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
}

void loop()
{   
  delay(1);
  if (digitalRead(4) == LOW){
    if (wav->loop()){
      if (digitalRead(4) == HIGH) {
        if (!wav->loop()) wav->stop();
      }
    } else {
      digitalWrite(5, LOW);
      ESP.deepSleep(5e6);
    }
  } else {
    digitalWrite(5, LOW);
    Serial.println("Trap has been triggered");
    ESP.deepSleep(60e6);
  }
}

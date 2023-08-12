#include <Arduino.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"

#include "viola.h"

#define MOSFET D8

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;

bool waitSeconds(int sec) {
  digitalWrite(MOSFET, LOW);
  ESP.deepSleep(sec * 1e6);
  return false;
}

void setup()
{
  Serial.begin(9600);

  pinMode(MOSFET, OUTPUT);

  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
  Serial.println("run");
  
  delay(100);
}

void loop()
{
  digitalWrite(MOSFET, HIGH);
  if (wav->loop()) {
      if (!wav->loop()) wav->stop();
  } else {
    Serial.println("ran");
    waitSeconds(3);
  }
}

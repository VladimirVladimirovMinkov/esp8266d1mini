#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"

#include "viola.h"

AudioGeneratorWAV *wav[2];
AudioFileSourcePROGMEM *file[2];
AudioOutputI2S *out;
AudioOutputMixer *mixer;
AudioOutputMixerStub *stub[2];

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(4, INPUT_PULLUP);
  
  WiFi.mode(WIFI_OFF); //turns wifi off for power saving
  Serial.begin(115200);
  delay(100);

  digitalWrite(2, LOW);

  // defines viola.h for playing 
  audioLogger = &Serial;
  file[0] = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2S();
  mixer = new AudioOutputMixer(32, out);
  stub[0] = mixer->NewInput();
  stub[0]->SetGain(0.3);
  wav[0] = new AudioGeneratorWAV();
  wav[0]->begin(file[0], stub[0]);
}

void loop()
{
  int sleep = 1;
  static bool go = false;

  if (wav[0]->isRunning()) {
    if (!wav[0]->loop()) { wav[0]->stop(); stub[0]->stop(); }
  }

  if (digitalRead(4) == 0)
  {
    sleep = sleep*4;
    if (sleep > 3600) sleep = 1;
  }

  if (millis() > 5000) {
    if (!go) {
      // defines viola.h for playing 
      stub[1] = mixer->NewInput();
      stub[1]->SetGain(0.4);
      wav[1] = new AudioGeneratorWAV();
      file[1] = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
      wav[1]->begin(file[1], stub[1]);
      go = true;
    }

    if (wav[1]->isRunning()) { 
      // keeps program here until end of viola.h then stops playing
      if (!wav[1]->loop()) { wav[1]->stop(); stub[1]->stop();} 

      // shuts down amplifier
      pinMode(14, OUTPUT);
      delay(100);
      digitalWrite(14, LOW);
      delay(100);

      // restarts the esp after x amount of microseconds or 1^-6 seconds
      ESP.deepSleep(sleep * 1e6);
    }
  }
}
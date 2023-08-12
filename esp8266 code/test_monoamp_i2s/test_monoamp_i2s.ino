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

#define D5 AmpSDpin

void setup()
{
  WiFi.mode(WIFI_OFF); //turns wifi off for power saving
  Serial.begin(115200);
  delay(100);
  Serial.print("WAV start\n");

  // defines viola.h for playing 
  audioLogger = &Serial;
  file[0] = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2S();
  mixer = new AudioOutputMixer(32, out);
  stub[0] = mixer->NewInput();
  stub[0]->SetGain(0.3);
  wav[0] = new AudioGeneratorWAV();
  wav[0]->begin(file[0], stub[0]);
  Serial.print("starting 0\n");
}

void loop()
{
  static uint32_t start = 0;
  static bool go = false;
  
  if (!start) start = millis();

  if (wav[0]->isRunning()) {
    if (!wav[0]->loop()) { wav[0]->stop(); stub[0]->stop(); Serial.print("stopping 0\n"); }
  }

  if (millis()-start > 3000) {
    if (!go) {
      // defines viola.h for playing 
      Serial.printf("starting 1\n");
      stub[1] = mixer->NewInput();
      stub[1]->SetGain(0.4);
      wav[1] = new AudioGeneratorWAV();
      file[1] = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
      wav[1]->begin(file[1], stub[1]);
      go = true;
    }
    if (wav[1]->isRunning()) { 
      // keeps program here until end of viola.h then stops playing
      if (!wav[1]->loop()) { wav[1]->stop(); stub[1]->stop(); Serial.print("stopping 1\n");} 

      // brings amplifier pin low
      pinMode(AmpSDpin, OUTPUT); 
      digitalWrite(AmpSDpin, LOW); 
      delay(10); 

      // restarts the esp after x amount of microseconds or 1^-6 seconds
      ESP.deepSleep(3e6); 
    }
  }

}

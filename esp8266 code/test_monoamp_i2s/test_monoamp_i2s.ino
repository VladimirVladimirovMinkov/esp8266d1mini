#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP_EEPROM.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"

#include "MouseSounds.h"

AudioGeneratorWAV *wav[2];
AudioFileSourcePROGMEM *file[2];
AudioOutputI2S *out;
AudioOutputMixer *mixer;
AudioOutputMixerStub *stub[2];

int sleep = -1;
#define btn_input 5   // D1
#define btn_output 0  // D3
#define amp_sd 14     // D5
#define indicatorLED 13  // D7

void setup() {
  Serial.begin(74880);
  delay(50);
  EEPROM.begin(1);

  pinMode(btn_output, OUTPUT);
  pinMode(btn_input, INPUT_PULLUP);
  pinMode(indicatorLED, INPUT);

  WiFi.mode(WIFI_OFF);  //turns wifi off for power saving

  digitalWrite(btn_output, LOW);

  // defines viola.h for playing
  audioLogger = &Serial;
  file[0] = new AudioFileSourcePROGMEM(viola, sizeof(viola));
  out = new AudioOutputI2S();
  mixer = new AudioOutputMixer(32, out);
  stub[0] = mixer->NewInput();
  stub[0]->SetGain(0.3);
  wav[0] = new AudioGeneratorWAV();
  wav[0]->begin(file[0], stub[0]);

  EEPROM.get(0, sleep);
  delay(10);
  Serial.println(sleep);
  
  if ((sleep < 1) || (sleep > 15))
    sleep = 1;
  
  Serial.println(sleep);
  delay(10);
}

void loop() {
  bool go = false;

  if (wav[0]->isRunning()) {
    go = true;
    if (!wav[0]->loop()) {
      wav[0]->stop();
      stub[0]->stop();
      go = false;
    }
  }

  if (digitalRead(btn_input) == 0) {
    if (sleep == 1) sleep = 2;
    else if (sleep == 2) sleep = 5;
    else if (sleep == 5) sleep = 10;
    else if (sleep == 10) sleep = 15;
    else sleep = 1;
    
    Serial.println(sleep);
    EEPROM.put(0, sleep);
    
    delay(10);
    
    EEPROM.commit();

    pinMode(amp_sd, OUTPUT);
    delay(100);
    digitalWrite(amp_sd, LOW);
    
    for (int i = 0; i < sleep; i++)
    {
      digitalWrite(indicatorLED, HIGH);
      delay(100);
      digitalWrite(indicatorLED, LOW);
      delay(100);
    }
  }

  if ((millis() > 5000) || (go == false)) {
    // shuts down amplifier
    pinMode(amp_sd, OUTPUT);
    delay(100);
    digitalWrite(amp_sd, LOW);
    delay(100);

    // restarts the esp after x amount of microseconds or 1^-6 seconds
    ESP.deepSleep(sleep * 60e6);
  }
}

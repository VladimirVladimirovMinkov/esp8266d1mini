bool ds_en = false;
bool dht_en = false;
bool bme_en = false;

// shared libraries
#include <dummy.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// enable/dissable librarries

#if dht_en
#include <DHTesp.h>
// setup dht to esp communication
DHTesp dht;
#endif

#if ds_en
#include <DallasTemperature.h>
#define ONE_WIRE_PIN_DS 2 // d4
OneWire oneWire_ds(ONE_WIRE_PIN_DS);
DallasTemperature ds_sensor(&oneWire_ds);
#endif

#if bme_en
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>

//#define BME_SCK 14
//#define BME_MISO 12
//#define BME_MOSI 13
//#define BME_CS 15
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime;
#endif

// wifi connection
#define WIFI_SSID "homem1"
#define WIFI_PASS "Yanaminkovladkodestroytheworld1"

// mqtt and domoticz information and connection
#define AIO_SERVER      "192.168.1.241"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "vladi"
#define AIO_KEY         ""
#define AIO_CHANNEL     "domoticz/in"
#define AIO_IDX         43

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called mqtt_channel for publishing.
// Notice MQTT paths for domoticz follows: domoticz/<in or out>
Adafruit_MQTT_Publish mqtt_channel = Adafruit_MQTT_Publish(&mqtt, AIO_CHANNEL);

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/switch_vlad");

// setup webserver
ESP8266WebServer webserver(80);

#if ds_en
int period_ds = 60000;
int time_now_ds = 00000;
float temp_ds = -120.0
#endif

#if dht_en
int period_dht = 60000;
int time_now_dht = 00000;
float temp_dht = -110.9
float humid_dht = -1.1
#endif

#if bme_en
int period_bme = 60000;
int time_now_bme = 00000;
float temp_bme = -110
float humid_bme = -1.2
float pressure_bme = -1.0
#endif

void MQTT_connect();

void rootPage() {
  // creates the home page
  String res; 
  res += "Vlad Api \n";
  
  #if dht_en
  res += "/current_dht \n";
  res += "/dht \n";
  #endif
  
  #if ds_en
  res += "/current_ds \n";
  res += "/ds \n";
  #endif
  
  #if bme_en
  res += "/current_bme \n"
  res += "/bme \n";
  #endif
  
  webserver.send(200, "text/plain", res);
}

void dsTemp() { 
  // publishes the cached temprature from ds
  webserver.send(200, "text/plain", "temprature " + String(temp_ds));
}

void setup() {

}

void loop() {

}

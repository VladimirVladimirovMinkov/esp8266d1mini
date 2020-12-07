// librarys to be used
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// definitions for use
#define ONE_WIRE_PIN_SB 2
#define ONE_WIRE_PIN_DHT 15
#define WIFI_SSID "homem1"
#define WIFI_PASS "Yanaminkovladkodestroytheworld1"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// wifi/mqt/tcp values for connection verification
#define AIO_SERVER      "192.168.1.241" // prfered ip
#define AIO_SERVERPORT  1883                   // use 8883 for SSL, 1883 for mqtt/tcp to domoticz
#define AIO_USERNAME    "vladi"
#define AIO_KEY         ""


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called sensor1 for publishing.
// Notice MQTT paths for domoticz follow the form: domoticz/<in or out>
Adafruit_MQTT_Publish sensor1 = Adafruit_MQTT_Publish(&mqtt, "domoticz/in");

// Setup a feed called switch_term for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/switch_term");

// initiate the webserver on which port
ESP8266WebServer webserver(80);
// initiate one wire communication
OneWire oneWireSB(ONE_WIRE_PIN_SB);
OneWire oneWireDHT(ONE_WIRE_PIN_DHT);
// initiate sensors to communicate under one wire communication
DallasTemperature sensor_sb(&oneWireSB);
DallasTemperature sensor_dht(&oneWireDHT);

// initiate psuedo threding
int period = 6000;
int time_now = 0;

// initiate debug values for temprature
float temp_sb = -1;
String dis_temp = "-2";
float temp_dht = -3;


// connects to mqtt
void MQTT_connect();

// this is the root page that will be called when the ip is typed in
void rootPage() {
  String res; 
  res += "Vlad Api \n";
  res += "/temp \n";
  res += "/hunidity \n"; 
  webserver.send(200, "text/plain", res);
}

// this will send the cached value of temprature as a page when /temp is applyed to ip
void readTemp() { 
  webserver.send(200, "text/plain", dis_temp);
}

// this will read current temp and send it as a page when /currenttemp is applyed to ip
void readCurrentTemp() { 
  sensor_dht.requestTemperatures();
  sensor_sb.requestTemperatures();
  temp_dht = sensor_sb.getTempCByIndex(0);
  dis_temp = String(temp_sb);
  dis_temp += ", ";
  dis_temp += String(temp_dht);
  webserver.send(200, "text/plain", dis_temp);
}

// when an unknown modifier is added to ip this page will be sent
void notfoundPage(){ 
  webserver.send(404, "text/plain", "404: Not found"); 
}

void setup() {
  // begin serial
  Serial.begin(115200);
  Serial.println();

  // will connect to specified router and retry every 100 miliseconds to connect if a connecion hasn't been made
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(100); }

  // will put this is serrial for debug
  Serial.println(F("Adafruit MQTT demo"));
  Serial.print("Connected! IP address: ");
  // local ip on debug (should try to get it to print external ip aswell)
  Serial.println(WiFi.localIP());

  // the pages when being called
  webserver.on("/", rootPage);
  webserver.on("/temp", readTemp);
  webserver.on("/currenttemp", readCurrentTemp);
  webserver.onNotFound(notfoundPage);
  webserver.begin();
}

void loop(void){ 
  // when connected will sort the client
  webserver.handleClient();

  // will check if enough time has passed since last cached temprature
  if (millis() - time_now > period) {
    time_now = millis();
    sensor_sb.requestTemperatures();
    temp_sb = sensor_sb.getTempCByIndex(0);

    // sets up the value that is to be sent under mqtt to domoticz in a format for updating virtual sensor
    dis_temp = "{\"idx\":42, \"nvalue\":0, \"svalue\":\"";
    dis_temp += String(temp_sb);
    dis_temp += "\"}";

    // arrays temprature to be recognised by domoticz as virtual thermal sensor1
    //idx is identification, nvalue is spare value, svalue is value of sensor
    char char_temp[50];
    //sprintf(char_temp,"{\"idx\":42, \"nvalue\":0, \"svalue\":\"%.2f\"} ", temp);
    dis_temp.toCharArray(char_temp, 50);

    // will state in debug whether it was published to mqtt or nor
    if (! sensor1.publish(char_temp)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  // debug to know con necting
  Serial.print("Connecting to MQTT... ");

  // number of times to retry
  uint8_t retries = 3;

  // will retry connecting until there are no more retrys and will require resetting
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  // debug so you know it has connected
  Serial.println("MQTT Connected!");
}

// libaries used
#include <dummy.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ds18b20 and DHT22 one wire communication wires
#define ONE_WIRE_PIN_DS 2 // d4
#define ONE_WIRE_PIN_DHT 0 // d3

// wifi connection
#define WIFI_SSID "homem1"
#define WIFI_PASS "Yanaminkovladkodestroytheworld1"

// mqtt and domoticz information and connection
#define AIO_SERVER      "192.168.1.241"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "vladi"
#define AIO_KEY         ""

// set dhtesp communication to dht
DHTesp dht;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called sensor1 for publishing.
// Notice MQTT paths for domoticz follows: domoticz/<in or out>
Adafruit_MQTT_Publish sensor1 = Adafruit_MQTT_Publish(&mqtt, "domoticz/in");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/switch_vlad");

// setup webserver
ESP8266WebServer webserver(80);

// sets up one wire communication with ds pin
OneWire oneWire_ds(ONE_WIRE_PIN_DS);

// setsup the ds18b20 on the onewire communication
DallasTemperature ds_sensor(&oneWire_ds);

// time keeping variables
int period_ds = 60000;
int time_now_ds = 0;
int period_dht = 60000;
int time_now_dht = 0;

// setsup debug values that are impossible for ds_sensor used
float temp_ds = -120;
String dis_temp_ds = "-20";
float h = -50;
float t = -60;

void MQTT_connect();

void rootPage() {
  // creates the home page
  String res; 
  res += "Vlad Api \n";
  res += "/current_dht \n";
  res += "/current_ds \n";
  res += "/ds \n";
  res += "/dht \n";
  webserver.send(200, "text/plain", res);
}

void readTemp() { 
  // publishes the cached temprature from ds
  webserver.send(200, "text/plain", "temprature " + String(temp_ds));
}

void readCurrent() { 
  // gets current temprature and humidity and publishes it to web page
  h = dht.getHumidity();
  t = dht.getTemperature();
  String dis = "Temprature: " + String(t) + " Humidity: " + String(h); 
  webserver.send(200, "text/plain", dis);
}

void dht22() {
  webserver.send(200, "text/plain", "Temprature: " + String(t) + " Humidity: " + String(h));
}

void dsCurrent() {
  // reads ds and publishes to web page
  ds_sensor.requestTemperatures();
  float temp = ds_sensor.getTempCByIndex(0);
  webserver.send(200, "text/plain", String(temp));
}

void notfoundPage(){
  // a debug message to know that this is not a page that exsists
  webserver.send(404, "text/plain", "404: Not found"); 
}

void setup() {
  // initialise the serrial connection
  Serial.begin(115200);
  Serial.println();

  // setup pin communication
  dht.setup(ONE_WIRE_PIN_DHT, DHTesp::DHT22);

  // sets up the wifi connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(100); }

  // debug messages to show wifi connection is running properly
  Serial.println(F("Adafruit MQTT demo"));
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // web pages displayed on request
  webserver.on("/", rootPage);
  webserver.on("/ds", readTemp);
  webserver.on("/dht", dht22);
  webserver.on("/current_dht", readCurrent);
  webserver.on("/current_ds", dsCurrent);

  // web page not found web page
  webserver.onNotFound(notfoundPage);

  // begins teh web server
  webserver.begin();

  // Setup MQTT subscription for onoff feed.
  // mqtt.subscribe(&onoffbutton);
}

void loop(void){ 
  // begins webserver clients
  webserver.handleClient();

  // if enough time has passed it will send an update to domoticz from the dht22 sensor
  if (millis() - time_now_dht > period_dht) {
    time_now_dht = millis();

    // retrive temprature and humidity from dht22
    float h = dht.getHumidity();
    float t = dht.getTemperature();

    // debug hum and temp
    Serial.print("{\"humidity\": ");
    Serial.print(h);
    Serial.print(", \"temp\": ");
    Serial.print(t);
    Serial.print("}\n");
    
    // format into domoticz readable format
    String dis_temp_dht = "{\"idx\":43, \"nvalue\":0, \"svalue\":\"";
    dis_temp_dht += String(t);
    dis_temp_dht += ";";
    dis_temp_dht += String(h);
    dis_temp_dht += ";";
    dis_temp_dht += "1";
    dis_temp_dht += "\"}";

    Serial.print(dis_temp_dht);

    // turns things into a char array to be used for domoticz
    char char_temp[50];
    //sprintf(char_temp,"{\"idx\":43, \"nvalue\":0, \"svalue\":\"%.2f\"} ", temp);
    dis_temp_dht.toCharArray(char_temp, 50);

    // will publish the temprature and return if they have published it to location
    if (! sensor1.publish(char_temp)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }

  // if enough time has passed it will send an update to domoticz from the ds18b20 sensor
  if (millis() - time_now_ds > period_ds) {
    // updates last time the temprature is checked
    time_now_ds = millis();

    // gets current sensor values and sets the temprature under the variable temp
    ds_sensor.requestTemperatures();
    temp_ds = ds_sensor.getTempCByIndex(0);

    Serial.println(temp_ds);

    // formats the current temprature into a domoticz readable format
    // idx 42 is identification used in place of sensor1 or themal sensor
    dis_temp_ds = "{\"idx\":42, \"nvalue\":0, \"svalue\":\"";
    dis_temp_ds += String(temp_ds);
    dis_temp_ds += "\"}";

    // turns things into a char array to be used for domoticz
    char char_temp[50];
    //sprintf(char_temp,"{\"idx\":42, \"nvalue\":0, \"svalue\":\"%.2f\"} ", temp);
    dis_temp_ds.toCharArray(char_temp, 50);

    // will publish the temprature and return if they have published it to location
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

  //Adafruit_MQTT_Subscribe *subscription;
  //while ((subscription = mqtt.readSubscription(5000))) {
    //if (subscription == &onoffbutton) {
      //Serial.print(F("Got: "));
      //Serial.println((char *)onoffbutton.lastread);
    //}
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  
  // will retry connecting if no connection is made until a resonable amount of retrys are made
  uint8_t retries = 5;
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
  Serial.println("MQTT Connected!");
}

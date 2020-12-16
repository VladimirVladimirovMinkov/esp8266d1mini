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
//#ifdef DS_SENSOR
//# define ONE_WIRE_PIN_DHT 0 // d3
//# define ONE_WIRE_PIN_DS 2 // d4
//#else
# define ONE_WIRE_PIN_DHT 2 // d4
# define POWER_PIN_DHT 0 // d3
//#endif  

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

// set dhtesp communication to dht
DHTesp dht;

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

// setsup the ds18b20 on the onewire communication
//define DS_SENSOR 1
#ifdef DS_SENSOR
OneWire oneWire_ds(ONE_WIRE_PIN_DS);
DallasTemperature ds_sensor(&oneWire_ds);
#endif 

// time keeping variables
int period_ds = 60000;
int time_now_ds = 00000;
int period_dht = 60000;
int time_now_dht = 00000;

// setsup debug values that are impossible for ds18b20 and dht22
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
#ifdef DS_SENSOR  
  // reads ds and publishes to web page
  ds_sensor.requestTemperatures();
  float temp = ds_sensor.getTempCByIndex(0);
#else 
  float temp = -66.6;
#endif  
  webserver.send(200, "text/plain", String(temp));
}

void notfoundPage(){
  // a debug message to know that this is not a page that exsists
  webserver.send(404, "text/plain", "404: Not found"); 
}

void publish_mqtt(int idx, float temp, float humidity = -1) 
{
  // debug hum and temp
  Serial.print("{");
  if (humidity > 0) {
    Serial.print("\"humidity\": ");
    Serial.print(humidity);
    Serial.print(", ");
  }
  Serial.print("\"temp\": ");
  Serial.print(temp);
  Serial.print("}\n");
    
  // format into domoticz readable format
  String dis_temp_dht = "{\"idx\":";
  dis_temp_dht += String(idx);
  dis_temp_dht += ", \"nvalue\":0, \"svalue\":\"";
  dis_temp_dht += String(temp);
  if (humidity > 0) {
    dis_temp_dht += ";";
    dis_temp_dht += String(humidity);
    dis_temp_dht += ";";
    dis_temp_dht += "1";
  }
  dis_temp_dht += "\"}";

  Serial.print(dis_temp_dht);

  // turns things into a char array to be used for domoticz
  char char_temp[50];
  //sprintf(char_temp,"{\"idx\":43, \"nvalue\":0, \"svalue\":\"%.2f\"} ", temp);
  dis_temp_dht.toCharArray(char_temp, 50);

  // will publish the temprature and return if they have published it to location
  if (! mqtt_channel.publish(char_temp)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void web_publish_mqtt() {
  h = dht.getHumidity();
  t = dht.getTemperature();
  publish_mqtt(AIO_IDX, t, h);
  char dis[64];
  sprintf(dis, "temp: %f humidity: %f idx: %d", t, h, 43);
  
  webserver.send(200, "text/plain", dis);
}

void setup() {

  #ifndef DS_SENSOR
  digitalWrite(POWER_PIN_DHT, HIGH);
  pinMode(POWER_PIN_DHT, OUTPUT);
  #endif
  
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
  
  #ifdef DS_SENSOR
  webserver.on("/ds", readTemp);
  webserver.on("/current_ds", dsCurrent);
  #endif
  
  webserver.on("/dht", dht22);
  webserver.on("/current_dht", readCurrent);
  webserver.on("/publish", web_publish_mqtt);

  // web page not found web page
  webserver.onNotFound(notfoundPage);

  // begins teh web server
  webserver.begin();

  // Setup MQTT subscription for onoff feed.
  // mqtt.subscribe(&onoffbutton);
}

void loop(void){ 

  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  
  // begins webserver clients
  webserver.handleClient();

  // if enough time has passed it will send an update to domoticz from the dht22 sensor
  if (millis() - time_now_dht > period_dht) {
    time_now_dht = millis();

    // retrive temprature and humidity from dht22
    h = dht.getHumidity();
    t = dht.getTemperature();

    // publlishes and debug values temp and humidity optionaly
    publish_mqtt(AIO_IDX, t, h);
  }
  
  #ifdef DS_SENSOR
  // if enough time has passed it will send an update to domoticz from the ds18b20 sensor
  if (millis() - time_now_ds > period_ds) {
    // updates last time the temprature is checked
    time_now_ds = millis();

    // gets current sensor values and sets the temprature under the variable temp
    ds_sensor.requestTemperatures();
    temp_ds = ds_sensor.getTempCByIndex(0);
    
    int index = 42;
    publish_mqtt(index, temp_ds);
  }
  #endif

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

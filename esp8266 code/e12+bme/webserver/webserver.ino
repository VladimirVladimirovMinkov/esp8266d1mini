// esp wifi and webserver get
#include <ESP8266WebServer.h>

// wifi connection
#define WIFI_SSID "homem1"
#define WIFI_PASS "Yanaminkovladkodestroytheworld1"

// setup webserver
ESP8266WebServer webserver(80);

void rootPage() {
  // creates the home page
  String res; 
  res += "Vlad API \n";
  webserver.send(200, "text/plain", res);
}

void rootPage2() {
  // creates the home page
  String res; 
  res += "<h1>Vlad API<h1> \n";
  webserver.send(200, "text/html", res);
}

void notfoundPage(){
  // a debug message to know that this is not a page that exsists
  webserver.send(404, "text/plain", "404: Not found"); 
}

void setup() {
  // initialise the serrial connection
  Serial.begin(9600);
  Serial.println();

  // sets up the wifi connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(100); }

  // debug messages to show wifi connection is running properly
  Serial.println(F("webserver test demo"));
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // web pages displayed on request
  webserver.on("/", rootPage);
  webserver.on("/alt", rootPage2);

  // web page not found web page
  webserver.onNotFound(notfoundPage);

  // begins the web server
  webserver.begin();
}

void loop(void){
  // begins webserver clients
  webserver.handleClient();
}

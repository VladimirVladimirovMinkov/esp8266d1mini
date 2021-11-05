//============================================
//ESP8266 Web Server (HTML + CSS + JavaScript)
//============================================
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webpageCode.h";
//------------------------------------------
ESP8266WebServer server(80);
const char* ssid = "Redmi";
const char* password = "pass1234";
//------------------------------------------
void webpage()
{
  server.send(200,"text/html", webpageCode);
}
//=================================================================
void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println();
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  server.on("/", webpage);
  server.begin();
}
//=================================================================
void loop()
{
  server.handleClient();
}

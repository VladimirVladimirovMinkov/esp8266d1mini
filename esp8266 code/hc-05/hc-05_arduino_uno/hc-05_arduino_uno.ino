#include "SoftwareSerial.h"
SoftwareSerial RCcar(2, 3); // RX | TX 
SoftwareSerial Remote(4, 5); // RX | TX 
char holder = "empty";

void setup() 
{ 
 Serial.begin(9600); 
 RCcar.begin(9600);  //Baud Rate for AT-command Mode.
 Remote.begin(9600);  //Baud Rate for AT-command Mode.  
 Serial.println("***AT commands mode***"); 
} 

void loop() 
{ 
 //from bluetooth to Terminal. 
 if (RCcar.available()) {
   holder = RCcar.read();
   Remote.write(holder);
   Serial.write(holder);
 }
 //from termial to bluetooth 
 if (Remote.available()) {
   holder = Remote.read();
   RCcar.write(holder);
   Serial.write(holder);
 }
 if (Serial.available()) {
   holder = Serial.read();
   RCcar.write(holder);
   Remote.write(holder);
 }
}

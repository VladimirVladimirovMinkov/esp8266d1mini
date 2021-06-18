#include "SoftwareSerial.h"
SoftwareSerial RCcar(21, 22); // RX | TX 
SoftwareSerial Remote(21, 22); // RX | TX 

void setup() 
{ 
 Serial.begin(9600); 
 RCcar.begin(38400);  //Baud Rate for AT-command Mode.
 Remote.begin(38400);  //Baud Rate for AT-command Mode.  
 Serial.println("***AT commands mode***"); 
} 

void loop() 
{ 
 //from bluetooth to Terminal. 
 if (RCcar.available()) 
   Remote.write(RCcar.read());
   Serial.write(RCcar.read());
 //from termial to bluetooth 
 if (Remote.available()) 
   RCcar.write(Remote.read());
   Serial.write(Remote.read());
}

/*
 * Libs needed:
 * -ESP8266 libraries (for MaroWebServer)
 * -https://github.com/bblanchon/ArduinoJson.git version <= v5.8.0 (for MaroWebServer)
 * -https://github.com/Links2004/arduinoWebSockets (for MaroWebServer)
 * -https://github.com/kissste/ESP8266-Arduino-CC1101.git (for RemoteControl specific)
 *
 * furthermore:
 * -keeloq.cpp & keeloq.h copied from https://github.com/franksmicro/Arduino.git (for RemoteControl specific)
 *
 */

#include "MaroWebServer.h"
#include "Esp8266.h"
#include "RemoteControl.h"

MaroWebServer myWebServer;
Esp8266 myEsp(&myWebServer);//reference to webserver to serve as a GUI
RemoteControl rc(&myWebServer);//reference to webserver to serve as a GUI

void setup(void){
  PRINT_INIT(115200);
  PRINT("\n");
  DBGOUTPUT(true);
  delay(5000);
  myWebServer.begin();
  myEsp.begin();
  rc.begin();

}


void loop(void){
	myWebServer.handle();
	myEsp.handle(); //in case status changes of shall be reported/measured
	rc.handle();//for receiving data packets
}


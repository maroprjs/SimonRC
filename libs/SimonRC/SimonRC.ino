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

#include "HttpServer.h"
#include "WebUI_HttpServer.h"
#include "WebUI_Esp8266.h"
#include "RfBridge.h"
#include "WebUI_RfBridge.h"


HttpServer httpServer;
WebUI_HttpServer webUIHttpServer(&httpServer);
WebUI_Esp8266 webUIEsp(&httpServer);//reference to webserver to serve as a GUI
RfBridge rfBridge(httpServer.storage());//webserver's storage used (SPIFFS)
WebUI_RfBridge webUIRfBridge(&httpServer,&rfBridge);

void setup(void){
  PRINT_INIT(115200);
  PRINT("\n");
  DBGOUTPUT(true);
  delay(5000);
  httpServer.begin();
  webUIHttpServer.begin();
  webUIEsp.begin();
  rfBridge.begin();
  webUIRfBridge.begin();

}


void loop(void){
	httpServer.handle();
	webUIHttpServer.handle();
	webUIEsp.handle(); //in case status changes of shall be reported/measured
	rfBridge.handle();//for receiving data packets
	webUIRfBridge.handle();
}


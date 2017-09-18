/*
 * WebUI_Esp8266.h
 *
 *  Created on: 15.05.2017
 *      Author: maro
 */

#ifndef WEBUI_ESP8266_H_
#define WEBUI_ESP8266_H_



#include "HttpServer.h" //serves as GUI

class WebUI_Esp8266 {
	public:
		WebUI_Esp8266(HttpServer *);
		virtual ~WebUI_Esp8266();
		void begin();
		void handle();
	private:
		void flashFirmware();
		void restart();
		void reportEspStatus();
	private:
		HttpServer *_httpServer;

};



#endif /* WEBUI_ESP8266_H_ */

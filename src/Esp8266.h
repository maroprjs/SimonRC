/*
 * Esp8266.h
 *
 *  Created on: 15.05.2017
 *      Author: maro
 */

#ifndef ESP8266_H_
#define ESP8266_H_



#include "MaroWebServer.h" //serves as GUI

class Esp8266 {
	public:
		Esp8266(MaroWebServer *);
		virtual ~Esp8266();
		void begin();
		void handle();
	private:
		void flashFirmware();
		void restart();
		void reportEspStatus();
	private:
		MaroWebServer *_myWebServer;

};



#endif /* ESP8266_H_ */

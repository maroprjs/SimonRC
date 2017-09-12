/*
 * MaroWebServer.h
 *
 *  Created on: 01.09.2017
 *      Author: maro
 *
 *inspired by nailbuster
 *
 * needed libs:
 * -ESP8266 libraries
 * -https://github.com/bblanchon/ArduinoJson.git version <= v5.8.0
 * -https://github.com/Links2004/arduinoWebSockets
 */



#ifndef MAROWEBSERVER_H_
#define MAROWEBSERVER_H_

#include <Arduino.h>
#include "Debug.h"
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#include "Storage.h"
#include "User.h"
#include "WebGui.h"
#include "Network.h"


class MaroWebServer {
public:
	MaroWebServer();
	virtual ~MaroWebServer();
	void begin();
	void handle();
	ESP8266WebServer * http(){return _server;};
	Network * network(){return _network;};
	User * user(){return _user;};
	Storage * storage(){return _storage;};


private:
	ESP8266WebServer * _server;
	Network * _network;//(wifi of course) TODO: server&fs as input for wifi settings through web interface
	Storage * _storage;
	DNSServer * _dnsServer;
	User * _user;
	WebGui * _gui;

};

#endif /* MAROWEBSERVER_H_ */

/*
 * HttpServer.cpp
 *
 *  Created on: 01.09.2017
 *      Author: maro
 *
 *       *inspired by nailbuster
 *
 * needed Libs:
 * -ESP8266 libraries
 * -https://github.com/bblanchon/ArduinoJson.git version <= v5.8.0
 * -https://github.com/Links2004/arduinoWebSockets
 */

#include "HttpServer.h"


HttpServer::HttpServer() {
	_storage = new Storage();
	_server = new ESP8266WebServer(80);
	_dnsServer = new DNSServer();
	_network = new Network(_storage, _dnsServer);//(wifi of course) TODO: server&fs as input for wifi settings through web interface
	_user = new User(_server, _storage);
	//_gui = new WebGui(_server, _storage, _user, _network, _dnsServer);


}

void HttpServer::begin(){
	  _storage->begin();
	  _server->begin();
	  _network->begin();
	  //_gui->begin();
}

void HttpServer::handle(){
	  _server->handleClient();
	  _network->handle();
}



HttpServer::~HttpServer() {
	// TODO Auto-generated destructor stub
}


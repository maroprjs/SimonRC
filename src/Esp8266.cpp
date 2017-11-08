/*
 * Esp8266.cpp
 *
 *  Created on: 15.05.2017
 *      Author: maro
 */

#include "Esp8266.h"

Esp8266::Esp8266(MaroWebServer *server) {
	_myWebServer = server;

}

Esp8266::~Esp8266() {
	// TODO Auto-generated destructor stub
}

void Esp8266::begin(){
	_myWebServer->http()->on("/restartesp", std::bind(&Esp8266::restart, this));
	_myWebServer->http()->on("/flashupdate", HTTP_POST, [this]() {_myWebServer->http()->send(200, "text/html", (Update.hasError()) ? "FAIL" : "<a href=\"/\">OK Restarting....wait a few seconds, then click here!</a>"); ESP.restart();}, std::bind(&Esp8266::flashFirmware, this));
	_myWebServer->http()->on("/espstatus", std::bind(&Esp8266::reportEspStatus, this));
}

void Esp8266::handle(){
	//TODO: should here be some events implemented? e.g.: pin state change or other measurements
}

void Esp8266::flashFirmware(){

	if (_myWebServer->user()->isAdmin() == false) return;
	// handler for the file upload, get's the sketch bytes, and writes
	// them through the Update object
	_myWebServer->network()->stopDNS();
	HTTPUpload& upload = _myWebServer->http()->upload();
	if (upload.status == UPLOAD_FILE_START) {
		//TODO: check if this is needed: WiFiUDP::stopAll();
		//TODO: make this semaphpre MyWebServer.OTAisflashing = true;
		PRINTLN("Update: " + upload.filename);
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if (!Update.begin(maxSketchSpace)) {//start with max available size
			Update.printError(Serial);
		}
	}
	else if (upload.status == UPLOAD_FILE_WRITE) {
		PRINT(".");
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);

		}
	}
	else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) { //true to set the size to the current progress
			PRINTLN("Update Success: \nRebooting...\n"+ String(upload.totalSize));

		}
		else {
			Update.printError(Serial);
		}
	}
	else if (upload.status == UPLOAD_FILE_ABORTED) {
		Update.end();
		PRINTLN("Update was aborted");
		//TODO: make this semaphpre MyWebServer.OTAisflashing = false;
	}
	delay(0);

};

void Esp8266::restart() {

	if (_myWebServer->user()->isAdmin() == false) return;
	_myWebServer->http()->send(200, "text/html", "<a href=\"/\">Restarting Device....wait a few seconds, then click here!</a>");
	delay(100);
	ESP.restart();

}


void Esp8266::reportEspStatus(){

	PRINTLN("reportEspStatus called ");
	String json = "{";
	json += "\"heap\":"+String(ESP.getFreeHeap());
	json += ", \"analog\":"+String(analogRead(A0));
	json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
	json += "}";
	_myWebServer->http()->send(200, "text/json", json);
	json = String();
}

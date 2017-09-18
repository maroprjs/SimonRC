/*
 * WebUIRfBridge.h
 *
 *  Created on: 18.09.2017
 *      Author: pionet
 */

#ifndef WEBUI_RFBRIDGE_H_
#define WEBUI_RFBRIDGE_H_
#include "HttpServer.h"
#include "RfBridge.h"

class WebUI_RfBridge {
public:
	WebUI_RfBridge(HttpServer *,RfBridge*);
	virtual ~WebUI_RfBridge();
	void begin();
	void handle();
private:
	void handleWebGuiHttp();
private:
	HttpServer * _httpServer;
	RfBridge * _rfBridge;
};

#endif /* WEBUI_RFBRIDGE_H_ */

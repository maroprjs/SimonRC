/*
 * WebUIRfBridge.cpp
 *
 *  Created on: 18.09.2017
 *      Author: pionet
 */

#include "WebUI_RfBridge.h"
#include "Debug.h"

WebUI_RfBridge::WebUI_RfBridge(HttpServer *server,RfBridge* rfBridge) {
	_httpServer = server;
	_rfBridge = rfBridge;

}

WebUI_RfBridge::~WebUI_RfBridge() {
	// TODO Auto-generated destructor stub
}

void WebUI_RfBridge::begin() {
	// register to WebGui
	_httpServer->http()->on("/rc", std::bind(&WebUI_RfBridge::handleWebGuiHttp, this));

	//TODO: register for web socket as well?

}

void WebUI_RfBridge::handle(){

	//TODO: What'll be here?
}


/**
 *
 *  http input example:
 *		scope=rc_btn&groupIdx=0&chIdx1=0&chIdx2=2&btn=up
 *		scope=rc_btn&groupIdx=0&chIdx1=0&btn=up
 */
void WebUI_RfBridge::handleWebGuiHttp(){

	String reply = "{\"status\":";

	//handle server requests

	if (_httpServer->http()->args() == 0) return;
	PRINT(" handleWebGuiHttp() http scope: ");PRINTLN(_httpServer->http()->arg("scope"));
	if(_httpServer->http()->arg("scope") == "rc_btn"){


		//prepare/activate requested group for sending:

		ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();//TODO: validity check!
		PRINT("activating group with index: ");PRINTLN(grpIdx);
		//_channelGroupVector[grpIdx]->activateChannelGroup();
		String btn_pressed = _httpServer->http()->arg("btn");
		//_channelGroupVector[grpIdx]->btn(btn);
		_rfBridge->activateChannelGroup(grpIdx,btn_pressed);


		//prepare/activate requested channels within the group for sending:

		int i = 1;
		String chIdx = _httpServer->http()->arg("chIdx1");
		//std::vector<Channel*>::const_iterator channelIterator;//TODO: delete this if first solution works
		//Channel* chPtr = NULL;
		while (chIdx != "")
		{
			Channel::channelIdx_t idx = chIdx.toInt();//_httpServer->http()->arg("chIdx").toInt();
			PRINT("requested chIdx in this group: ");PRINTLN(idx);
			//chPtr = _channelGroupVector[grpIdx]->channelVector[idx];
			//PRINTLN(chPtr->active());
			//PRINTLN(chPtr->address());
			//PRINTLN(chPtr->groupIdx());
			//PRINTLN(chPtr->idx());
			//chPtr->activateChannel();
			_rfBridge->activateChannel(grpIdx, idx);
			i++;
			chIdx = _httpServer->http()->arg("chIdx" + String(i));
			PRINT("next chIdx: ");PRINTLN(chIdx);
			//btn = _httpServer->http()->arg("btn" +  String(i));<-several btn's maybe for later use
			//PRINT("next btn: ");PRINTLN(btn);
		}
		//initiate transmission (includes code and packet assembly)
		PRINTLN("calling transmit now");
		//_transceiver->transmit();//TODO: transmit only when channel collection/activation successful
		_rfBridge->commit();

	}
	reply = reply + "\"rc_ok\"" + "}";//TODO: send answer to ack/nack to server!!
	_httpServer->http()->send(200, "text/plain", reply );
	PRINTLN("");PRINTLN("Saving channel group settings: ");
	//saveRcChannelGroups();



}


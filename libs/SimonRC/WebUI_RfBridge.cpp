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
	_httpServer->http()->on("/rc", std::bind(&WebUI_RfBridge::_handleWebGuiHttp, this));

	//TODO: register for web socket as well?

}

void WebUI_RfBridge::handle(){

	//TODO: What'll be here?
}


/**
 *Web-Api for Remote Control-> handle server requests
 *
 *  http input examples:
 *
 *  	talk to single receiver:
 *  		http://<ip address>/rc?scope=rc_btn&groupIdx=0&chIdx1=0&btn=up (btn = up, down, stop, updown,learn , erase,copy)
 *		talk to group of receivers:
 *			http://<ip address>/rc?scope=rc_btn&groupIdx=0&chIdx1=0&chIdx2=2&btn=up
 *
 *
 *		list created receiver groups:
 *			http://<ip address>/rc?scope=rc_list_ch_grp
 *		list created receivers in a group:
 *			http://<ip address>/rc?scope=rc_list_channels&groupIdx=0
 *		create new group of receivers:
 *			http://<ip address>/rc?scope=rc_create_ch_grp&protocol=0 (0=simon, 1=somfy, 2=kopp) (optional: &alias="<my group name>", &cnt=<counter>, &address="<address>")
 *		modify group of receivers:
 *			http://<ip address>/rc?scope=rc_modify_ch_grp&groupIdx=0 (optional: &alias="<my group name>", &cnt=<counter>, &protocol=<protocol> , &address="<address>")
 *		delete group of receivers:
 *			http://<ip address>/rc?scope=rc_del_ch_grp&groupIdx=1
 *		delete last receiver(channel) within a group:
 *			http://<ip address>/rc?scope=rc_del_ch&groupIdx=0
 *		add a receiver(channel) to a group:
 *			http://<ip address>/rc?scope=rc_add_ch&groupIdx=0 (optional: &alias="<my receiver's name>" , &address="<address>")
 *
 */
void WebUI_RfBridge::_handleWebGuiHttp(){

	String reply = "{\"status\":";
	String reply_status_strg = "rc_ok";
	String reply_status_why = "";
	String reply_status_why_prefix = ",\"why\":\"";
	String reply_status_why_suffix = "\"";
	bool reply_status = true;
	String reply_data = "";

	//handle server requests

	if (_httpServer->http()->args() == 0) return;

	PRINT(" handleWebGuiHttp() http scope: ");PRINTLN(_httpServer->http()->arg("scope"));

	if(_httpServer->http()->arg("scope") == "rc_btn"){
		_rcButtonPressed();
		reply_status = _rfBridge->saveRcChannelGroups();//to save rolling variables
		if (!reply_status) reply_status_why = "failed to save channel group settings";
		//TODO: alternatively save at least counter value to EEPROM
	}
	else if(_httpServer->http()->arg("scope") == "rc_list_ch_grps"){
		reply_data = _rcListChannelGroups("rc_list_ch_grps");//TODO: differetiate between list&show

	}
	else if(_httpServer->http()->arg("scope") == "rc_create_ch_grp"){
		if (_httpServer->http()->hasArg("protocol")){
			_rcCreateChannelGroup();
			reply_status = _rfBridge->saveRcChannelGroups();
			if (reply_status == true) {reply_data = _rcListChannelGroups("rc_create_ch_grp");};//TODO: introduce "show"-method
			if (reply_status == false) {reply_status_why = "failed to save channel group settings";};
		}
		else{
			reply_status = false;
			reply_status_why = "no valid http request";
		};
	}
	else if(_httpServer->http()->arg("scope") == "rc_modify_ch_grp"){
		if (_httpServer->http()->hasArg("groupIdx")){
			_rcModifyChannelGroup(_httpServer->http()->arg("groupIdx").toInt());
			reply_status = _rfBridge->saveRcChannelGroups();
			if ( reply_status == true ) { reply_data = _rcListChannelGroups("rc_modify_ch_grp"); };//TODO: introduce show function
			if ( reply_status == false ) { reply_status_why = "failed to save channel group settings"; };
		}
		else{
			reply_status = false;
			reply_status_why = "no valid http request";
		};
	}
	else if(_httpServer->http()->arg("scope") == "rc_del_ch_grp"){
		if (_httpServer->http()->hasArg("groupIdx")){
			ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();
			PRINTLN("calling delete channel group function: ");
			PRINT("grpIdx: "); PRINTLN(grpIdx);
			reply_status = _rfBridge->deleteChannelGroup(grpIdx);
			reply_status = _rfBridge->saveRcChannelGroups();
			if ( reply_status == true ) { reply_data = _rcListChannelGroups("rc_del_ch_grp"); };
			if ( reply_status == false ) { reply_status_why = "couldn't find that group or failed to save channel group settings"; };
		}
		else{
			reply_status = false;
			reply_status_why = "no valid http request";
		};

	}
	else if(_httpServer->http()->arg("scope") == "rc_list_channels"){
		PRINTLN("calling list channels function: ");
		if (_httpServer->http()->hasArg("groupIdx")) {//list channels of a given group
			ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();
			reply_data = _rcListChannels(grpIdx,"rc_list_channels");
		}else{//list all channels in memory
			reply_data = _rcListChannels(UNDEFINED_HEX_BYTE,"rc_list_channels");
		}
	}else if(_httpServer->http()->arg("scope") == "rc_add_ch"){
		if (_httpServer->http()->hasArg("groupIdx")){
			ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();
			_rcAddChannel(grpIdx);
			reply_status = _rfBridge->saveRcChannels();
			if ( reply_status == true ) { reply_data = _rcListChannels(grpIdx,"rc_add_ch"); };
			if ( reply_status == false ) { reply_status_why = "failed to save channel settings"; };
		}
		else{
			reply_status = false;
			reply_status_why = "no valid http request";
		};
	}
	else if(_httpServer->http()->arg("scope") == "rc_del_ch"){
		if (_httpServer->http()->hasArg("groupIdx")){
			ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();
			PRINTLN("calling delete channel function: ");
			reply_status = _rfBridge->deleteChannel(grpIdx);//last channel in group
			reply_status = _rfBridge->saveRcChannels();
			if ( reply_status == true ) { reply_data = _rcListChannels(grpIdx,"rc_del_ch"); };
			if ( reply_status == false ) { reply_status_why = "couldn't find that channel or failed to save channel settings"; };
		}
		else{
			reply_status = false;
			reply_status_why = "no valid http request";
		};
	}
	else{
		reply_status = false;
		reply_status_why = "no valid http request";
	}

	if ( reply_status == false ){
		reply_status_strg = "rc_nok";
		reply_data = reply_status_why_prefix + reply_status_why + reply_status_why_suffix;
	}
	reply = reply + "\"" + reply_status_strg + "\"" + reply_data + "}";//TODO: send answer to ack/nack to server!!
	_httpServer->http()->send(200, "text/plain", reply );
	//PRINTLN("");PRINTLN("Saving channel group settings: ");
	//saveRcChannelGroups();


}

void WebUI_RfBridge::_rcButtonPressed(){
	//prepare/activate requested group for sending:
	ChannelGroup::groupIdx_t grpIdx = _httpServer->http()->arg("groupIdx").toInt();//TODO: validity check!
	PRINT("activating group with index: ");PRINTLN(grpIdx);
	String btn_pressed = _httpServer->http()->arg("btn");
	_rfBridge->activateChannelGroup(grpIdx,btn_pressed);

	//prepare/activate requested channels within the group for sending:
	int i = 1;
	String chIdx = _httpServer->http()->arg("chIdx1");
	while (chIdx != "")
	{
		Channel::channelIdx_t idx = chIdx.toInt();//_httpServer->http()->arg("chIdx").toInt();
		PRINT("requested chIdx in this group: ");PRINTLN(idx);
		_rfBridge->activateChannel(grpIdx, idx);
		i++;
		chIdx = _httpServer->http()->arg("chIdx" + String(i));
		PRINT("next chIdx: ");PRINTLN(chIdx);

	}

	//initiate transmission (includes code and packet assembly)
	PRINTLN("activating transmission now");
	_rfBridge->activateTransmission();

}

String WebUI_RfBridge::_rcListChannelGroups(String scope){
	String reply_data = "";
	PRINTLN("calling list channel groups function: ");
	reply_data = _rfBridge->listChannelGroups();
	if (reply_data != "{}"){//if no channel group created
		//reply_data = ", \"rc_list_ch_grp\":" + reply_data;
		reply_data = ", \"" + scope + "\":" + reply_data;
	}
	else{
		reply_data = "";
	}
	PRINTLN(reply_data);

	return reply_data;
}


String WebUI_RfBridge::_rcListChannels(ChannelGroup::groupIdx_t grpIdx,String scope){
	String reply_data = "";
	PRINTLN("calling list channels function: ");
	reply_data = _rfBridge->listChannels(grpIdx);
	if (reply_data != "{}"){//if no channel group created
		//reply_data = ", \"rc_list_channels\":" + reply_data;
		reply_data = ", \"" + scope + "\":" + reply_data;
	}
	else{
		reply_data = "";
	}
	PRINTLN(reply_data);

	return reply_data;
}

void WebUI_RfBridge::_rcCreateChannelGroup(){
	PRINTLN("create ch grp function called ");
	ChannelGroup::protocol_t protocol;
	String alias = UNDEFINED_STRG;
	String address = UNDEFINED_STRG;
	ChannelGroup::counter_t cnt = 0;
	protocol = (ChannelGroup::protocol_t)_httpServer->http()->arg("protocol").toInt();
	if (_httpServer->http()->hasArg("alias")) alias = _httpServer->http()->arg("alias");
	if (_httpServer->http()->hasArg("address")) address = _httpServer->http()->arg("address");
	if (_httpServer->http()->hasArg("cnt")) cnt = _httpServer->http()->arg("cnt").toInt();
	_rfBridge->createChannelGroup(protocol,alias,address,cnt);

}

void WebUI_RfBridge::_rcModifyChannelGroup(ChannelGroup::groupIdx_t grpIdx){
	PRINT("modify ch grp function called, grpIdx: ");PRINTLN(grpIdx);
	ChannelGroup::protocol_t protocol = ChannelGroup::UNDEFINED;
	String alias = UNDEFINED_STRG;
	String address = UNDEFINED_STRG;
	ChannelGroup::counter_t cnt = 0;
	if (_httpServer->http()->hasArg("protocol")) protocol = (ChannelGroup::protocol_t)_httpServer->http()->arg("protocol").toInt();
	if (_httpServer->http()->hasArg("alias")) alias = _httpServer->http()->arg("alias");
	if (_httpServer->http()->hasArg("address")) address = _httpServer->http()->arg("address");
	if (_httpServer->http()->hasArg("cnt")) cnt = _httpServer->http()->arg("cnt").toInt();
	_rfBridge->modifyChannelGroup(grpIdx, protocol, alias, address, cnt);

}

void WebUI_RfBridge::_rcAddChannel( ChannelGroup::groupIdx_t grpIdx ){
	PRINTLN("add channel function called: ");
	String alias = UNDEFINED_STRG;
	String address = UNDEFINED_STRG;
	if (_httpServer->http()->hasArg("alias")) alias = _httpServer->http()->arg("alias");
	if (_httpServer->http()->hasArg("address")) address = _httpServer->http()->arg("address");
	_rfBridge->addChannel(grpIdx, alias, address);
}

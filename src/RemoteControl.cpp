/*
 * RemoteControl.cpp
 *
 *  Created on: 31.08.2017
 *      Author: maro
 */

#include "RemoteControl.h"
#include <ArduinoJson.h>
#include "Channel.h"

RemoteControl::RemoteControl(MaroWebServer *server) {
	_myWebServer = server;
	_transceiver = new Transceiver();//TODO: give socket here, so receiving function can report data
}

RemoteControl::~RemoteControl() {
	// TODO Auto-generated destructor stub
}

void RemoteControl::begin(){

	_transceiver->begin();

	//read channel config
	if (loadRcConfig() == false){
		PRINTLN("in RC::begin(): rcChannels.json NOT found ");
	}else{
		_transceiver->registerChannelGroups(_channelGroupVector);
	}


	// register to WebGui
	_myWebServer->http()->on("/rc", std::bind(&RemoteControl::handleWebGuiHttp, this));

	//TODO: register for web socket as well?

}

void RemoteControl::handle(){

	_transceiver->handle();
}

/**
 *
 *  http input example:
 *		scope=rc_btn&groupIdx=0&chIdx1=0&chIdx2=2&btn=up
 *		scope=rc_btn&groupIdx=0&chIdx1=0&btn=up
 */
void RemoteControl::handleWebGuiHttp(){

	String reply = "{\"status\":";

	//handle server requests

	if (_myWebServer->http()->args() == 0) return;
	PRINT(" handleWebGuiHttp() http scope: ");PRINTLN(_myWebServer->http()->arg("scope"));
	if(_myWebServer->http()->arg("scope") == "rc_btn"){


		//prepare/activate requested group for sending:

		ChannelGroup::groupIdx_t grpIdx = _myWebServer->http()->arg("groupIdx").toInt();//TODO: validity check!
		PRINT("activating group with index: ");PRINTLN(grpIdx);
		_channelGroupVector[grpIdx]->activateChannelGroup();
		String btn = _myWebServer->http()->arg("btn");
		_channelGroupVector[grpIdx]->btn(btn);


		//prepare/activate requested channels within the group for sending:

		int i = 1;
		String chIdx = _myWebServer->http()->arg("chIdx1");
		//std::vector<Channel*>::const_iterator channelIterator;//TODO: delete this if first solution works
		Channel* chPtr = NULL;
		while (chIdx != "")
		{
			Channel::channelIdx_t idx = chIdx.toInt();//_myWebServer->http()->arg("chIdx").toInt();
			PRINT("requested chIdx in this group: ");PRINTLN(idx);
			chPtr = _channelGroupVector[grpIdx]->channelVector[idx];
			PRINTLN(chPtr->active());
			PRINTLN(chPtr->address());
			PRINTLN(chPtr->groupIdx());
			PRINTLN(chPtr->idx());
			chPtr->activateChannel();
			i++;
			chIdx = _myWebServer->http()->arg("chIdx" + String(i));
			PRINT("next chIdx: ");PRINTLN(chIdx);
			//btn = _myWebServer->http()->arg("btn" +  String(i));<-several btn's maybe for later use
			//PRINT("next btn: ");PRINTLN(btn);
		}
		//initiate transmission (includes code and packet assembly)
		PRINTLN("calling transmit now");
		_transceiver->transmit();//TODO: transmit only when channel collection/activation successful


	}
	reply = reply + "\"rc_ok\"" + "}";//TODO: send answer to ack/nack to server!!
	_myWebServer->http()->send(200, "text/plain", reply );
	PRINTLN("");PRINTLN("Saving channel group settings: ");
	saveRcChannelGroups();



}





bool RemoteControl::loadRcConfig()
{
	bool retVal = true;

	if (loadRcChannelGroups() == false) return false;//must be called before channels are loaded
	if (loadRcChannels() == false) return false;

	return retVal;
}

bool RemoteControl::saveRcChannelGroups()
{
	bool retVal = false;
	DynamicJsonBuffer jsonBuffer;
	String content = "";
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	ChannelGroup* chGrpPtr = NULL;
	JsonObject& jChannelGroups = jsonBuffer.createObject();


	//JsonArray& jChannelGroup;
	//jsonData

	chGrpIterator = _channelGroupVector.begin();
	int i = 1;
	while (chGrpIterator != _channelGroupVector.end()){
		PRINT("start serializing group: "); PRINTLN(i);
		chGrpPtr = *chGrpIterator;
		content = "";
		/*JsonArray& jsonChGrpsArray = jChannelGroupsObj.createNestedArray("channel_group"+String(i));
		JsonObject& jChannelGroup = jsonBuffer.createObject();
		chGrpPtr->serialize(jChannelGroup);
		jsonChGrpsArray.add(jChannelGroup);*/
		JsonObject& jsonChGrp = jChannelGroups.createNestedObject("channel_group"+String(i));
//		JsonObject& jChannelGroup = jsonBuffer.createObject();
		chGrpPtr->serialize(jsonChGrp);
//		chGrpPtr->serialize(jChannelGroup);
//		jsonChGrpsNObj["channel_group"+String(i)] = jChannelGroup;// = jChannelGroup;
		i++;
		chGrpIterator++;

		PRINTLN("group serialized");
	}
	jChannelGroups.printTo(content);
	PRINT("in RC::saveRcChannelGroups() content: ");PRINTLN(content);
	_myWebServer->storage()->save("/rcChannelGroups.json", content);
	return retVal;
}

bool RemoteControl::loadRcChannelGroups()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;

	values = _myWebServer->storage()->open("/rcChannelGroups.json");
	PRINT("in RC::loadRcChannelGroups() values: ");PRINTLN(values);
	JsonObject& rcChannelGroupsJson = jsonBuffer.parseObject(values);  //parse weburl

	int i = 1;
	while ((rcChannelGroupsJson.success() == true) && (rcChannelGroupsJson.containsKey("channel_group" + String(i)) == true))
	{
		PRINT("in RC::loadRcChannelGroups(): rcChannelGroups.json found; check group");PRINTLN(String(i));
		String chGrpJsonStrg = rcChannelGroupsJson["channel_group" + String(i)];
		JsonObject& chGrpJson = jsonBuffer.parseObject(chGrpJsonStrg);
		i++;

		if (chGrpJson.success() &&
				chGrpJson.containsKey("groupIdx") && chGrpJson.containsKey("alias") &&
				chGrpJson.containsKey("address") && chGrpJson.containsKey("last_btn") &&
				chGrpJson.containsKey("counter") && chGrpJson.containsKey("protocol"))
		{
			PRINT("in loadRcChannelGroups(): group");PRINT(String(i-1));PRINTLN(" json is valid. ");
			ChannelGroup*  chGrp = new ChannelGroup(chGrpJsonStrg);
			_channelGroupVector.push_back(chGrp);
			retVal = true;
		}else{
			PRINTLN("in loadRcChannelGroups(): rcChannelGroups.json is NOT valid");
		}
	}

	return retVal;
}

bool RemoteControl::loadRcChannels()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;

	values = _myWebServer->storage()->open("/rcChannels.json");
	PRINT("in RC::loadRcChannels() values: ");PRINTLN(values);
	JsonObject& rcChannelsJson = jsonBuffer.parseObject(values);  //parse weburl

	int i = 1;
	while ((rcChannelsJson.success() == true) && (rcChannelsJson.containsKey("channel" + String(i)) == true))
	{
		PRINT("in RC::loadRcChannels(): rcChannels.json found; check channel");PRINTLN(String(i));
		String channelJsonStrg = rcChannelsJson["channel" + String(i)];
		PRINTLN(channelJsonStrg);
		JsonObject& channelJson = jsonBuffer.parseObject(channelJsonStrg);
		i++;
		PRINTLN(channelJson.success());
		if (channelJson.success() &&
			channelJson.containsKey("chIdx") && channelJson.containsKey("alias") &&
			channelJson.containsKey("address") && channelJson.containsKey("groupIdx"))
		{
			PRINT("in loadRcChannels(): channel");PRINT(String(i-1));PRINTLN(" json is valid. ");
			Channel*  ch =  new Channel(channelJsonStrg);
			PRINT("load groupIdx: ");PRINT(ch->groupIdx());PRINT(" ;from channelIdx: ");PRINTLN(ch->idx());
			_channelGroupVector[ch->groupIdx()]->channelVector.push_back(ch);//TODO: make validity check
			retVal = true;
		}else{
			PRINTLN("in loadRcChannels(): rcChannels.json is NOT valid");
		}
	}
PRINTLN("getting out");
	return retVal;
}


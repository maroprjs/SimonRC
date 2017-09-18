/*
 * RfBridge.cpp
 *
 *  Created on: 31.08.2017
 *      Author: maro
 */
#include "Arduino.h"
#include <ArduinoJson.h>
#include "Channel.h"
#include "lz.h"
#include "RCChannelsEmbed.h"
#include "RfBridge.h"
#include "Debug.h"

RfBridge::RfBridge(Storage *storage) {
	_storage = storage;
	_transceiver = new Transceiver();//TODO: give socket here, so receiving function can report data
}

RfBridge::~RfBridge() {
	// TODO Auto-generated destructor stub
}

void RfBridge::begin(){

	_transceiver->begin();

	//read channel config
	if (loadRcConfig() == false){
		PRINTLN("in RC::begin(): rcChannels.json NOT found ");
	}else{
		_transceiver->registerChannelGroups(_channelGroupVector);
	}


}

void RfBridge::handle(){

	_transceiver->handle();
}


void RfBridge::activateChannelGroup(ChannelGroup::groupIdx_t grpIdx,String btn_pressed){
	_channelGroupVector[grpIdx]->activateChannelGroup();
	_channelGroupVector[grpIdx]->btn(btn_pressed);
}

void RfBridge::activateChannel(ChannelGroup::groupIdx_t grpIdx,Channel::channelIdx_t idx){
	Channel* chPtr = _channelGroupVector[grpIdx]->channelVector[idx];
	chPtr->activateChannel();
	//PRINTLN(chPtr->active());
	//PRINTLN(chPtr->address());
	//PRINTLN(chPtr->groupIdx());
	//PRINTLN(chPtr->idx());
}

void RfBridge::commit(){
  _transceiver->transmit();//TODO: transmit only when channel collection/activation successful
  saveRcChannelGroups();
}





bool RfBridge::loadRcConfig()
{
	bool retVal = true;

	if (loadRcChannelGroups() == false) return false;//must be called before channels are loaded
	if (loadRcChannels() == false) return false;

	return retVal;
}

bool RfBridge::saveRcChannelGroups()
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
	_storage->save("/rcChannelGroups.json", content);
	_storage->saveLZ("/rcChannelGroupsXXX.json", content);
	return retVal;
}

bool RfBridge::loadRcChannelGroups()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;

	//values = _myWebServer->storage()->openLZ("/rcChannelGroups.json");
	values = _storage->open("/rcChannelGroups.json");
	if (values == ""){//file not found in flash
		PRINTLN("in RC::loadRcChannelGroups() using embedded!!!!! ");
		uint osize = sizeof(RC_CHANNEL_GRP) * 1.01;
		PRINT("embedded file size: ");PRINTLN(osize);
		unsigned char *myout = new unsigned char [osize];
		unsigned char myout2[sizeof(RC_CHANNEL_GRP)];
		LZ_Uncompress( myout, myout2,2 );

	}
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

bool RfBridge::loadRcChannels()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;

	values = _storage->open("/rcChannels.json");
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


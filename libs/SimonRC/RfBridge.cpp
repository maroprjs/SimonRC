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
//#include "RCChannelsEmbed.h"
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
		PRINTLN("in RC::begin(): configuration files missing or incomplete ");
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

void RfBridge::createChannelGroup(ChannelGroup::protocol_t protocol, String alias, String address, ChannelGroup::counter_t cnt){
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;
	ChannelGroup::groupIdx_t newGrpIdx = UNDEFINED_HEX_BYTE;


	//iterate all channel groups from rcChannelGroups.json.lz(initialized at startup) and get
	//next free group index
	while (chGrpIterator != _channelGroupVector.end()){
		//PRINTLN("while called");
		chGrpPtr = *chGrpIterator;
		if (chGrpPtr->protocol() == protocol) newGrpIdx = chGrpPtr->idx() + 1;
		chGrpIterator++;
	};

	//fill undefined values from template
	ChannelGroup* newChGrpPtr = new ChannelGroup();
	//TODO: should following initialization be done in transceiver codec?:
	if (newGrpIdx == UNDEFINED_HEX_BYTE) newGrpIdx = 0;//automatically, first channelGroup
	newChGrpPtr->idx(newGrpIdx);

	PRINTLN("calling transceiver: ");
	if (protocol == ChannelGroup::SIMON)_transceiver->simon->loadChannelGroupTemplate(newChGrpPtr,newGrpIdx);
	PRINT(newChGrpPtr->alias());//check if something happened

	if (alias != UNDEFINED_STRG) newChGrpPtr->alias(alias);//if manually given (default initialized in function prototype)

	if (address != UNDEFINED_STRG) newChGrpPtr->address(address);//if manually given (default initialized in function prototype)

	newChGrpPtr->counter(cnt);//if manually given (default initialized in function prototype)

	newChGrpPtr->protocol(protocol);

	newChGrpPtr->btn("");

	//add channel group to vector
	_channelGroupVector.push_back(newChGrpPtr);

}

bool RfBridge::modifyChannelGroup(ChannelGroup::groupIdx_t grpIdx, ChannelGroup::protocol_t protocol, String alias, String address, ChannelGroup::counter_t cnt){

	bool retVal = false;
	ChannelGroup::channelGroupVector_t::iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;
	ChannelGroup::groupIdx_t foundGrpIdx = UNDEFINED_HEX_BYTE;


	//find associated channel group from rcChannelGroups.json.lz(initialized at startup)
	while (chGrpIterator != _channelGroupVector.end()){
		//PRINTLN("while called");
		chGrpPtr = *chGrpIterator;
		if (chGrpPtr->idx() == grpIdx){
			foundGrpIdx = grpIdx;
			break;
		}
		chGrpIterator++;
	};

	if (foundGrpIdx == UNDEFINED_HEX_BYTE){
		PRINTLN("group not found");
		return retVal;//no group
	}
	else {
		retVal = true;
		PRINT("group found, grpIdx: ");PRINTLN(foundGrpIdx);
	}

	//fill record for channel group:
	if (alias != UNDEFINED_STRG) chGrpPtr->alias(alias);

	if (address != UNDEFINED_STRG) chGrpPtr->address(address);

	if (cnt != 0) chGrpPtr->counter(cnt);

	if (protocol != ChannelGroup::UNDEFINED) chGrpPtr->protocol(protocol);

	chGrpPtr->btn("");


	return retVal;

}


bool RfBridge::addChannel(ChannelGroup::groupIdx_t grpIdx, String alias, String address){
	bool retVal = false;
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;
	ChannelGroup::groupIdx_t foundGrpIdx = UNDEFINED_HEX_BYTE;

	//find associated channel group from rcChannelGroups.json.lz(initialized at startup)
	while (chGrpIterator != _channelGroupVector.end()){
		//PRINTLN("while called");
		chGrpPtr = *chGrpIterator;
		if (chGrpPtr->idx() == grpIdx){
			foundGrpIdx = grpIdx;
			break;
		}
		chGrpIterator++;
	};

	if (foundGrpIdx == UNDEFINED_HEX_BYTE){
		PRINTLN("no group created yet");
		return retVal;//create a group first!! - here retVal still false
	}
	else{
		PRINT("adding channel to group idx: ");PRINTLN(foundGrpIdx);
		//retVal = true;<- this value set below when protocol recognized successfully and max num of channels not exceeded
	}

	//find first free channel index
	Channel::channelIdx_t newChIdx = UNDEFINED_HEX_BYTE;
	Channel::channelVector_t::const_iterator chIterator;
	chIterator = chGrpPtr->channelVector.begin();
	Channel* chPtr = NULL;
	while (chIterator != chGrpPtr->channelVector.end()){//TODO: while loop not necessary
		//PRINTLN("while called");
		chPtr = *chIterator;
		newChIdx = chPtr->idx() + 1;
		chIterator++;
	};

	//create channel
	Channel* newChPtr = new Channel();
	if (newChIdx == UNDEFINED_HEX_BYTE) newChIdx = 0;//first channel
	newChPtr->idx(newChIdx);

	//load channel template
	//fill undefined values from template
	//PRINTLN("calling transceiver: ");
	if (chGrpPtr->protocol() == ChannelGroup::SIMON){
		retVal = _transceiver->simon->loadChannelTemplate(chGrpPtr, newChPtr, newChIdx);
	}
	//PRINT(newChPtr->alias());//check if something happened

	if ((alias != UNDEFINED_STRG) && (retVal == true)) newChPtr->alias(alias);//if manually given (default initialized in function prototype)

	if ((address != UNDEFINED_STRG) && (retVal == true)) newChPtr->address(address);//if manually given (default initialized in function prototype)

	//add channel to channel group
	if (retVal == true){
		chGrpPtr->channelVector.push_back(newChPtr);
		PRINT("Channel created alias: ");PRINT(newChPtr->alias());PRINT(" idx: ");PRINTLN(newChPtr->idx());
	}else{
		delete newChPtr;
	}

	return retVal;
}

/*
 * last channel in vector to be deleted
 */
bool RfBridge::deleteChannel(ChannelGroup::groupIdx_t grpIdx){
	bool retVal = false;
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;
	ChannelGroup::groupIdx_t foundGrpIdx = UNDEFINED_HEX_BYTE;

	//find associated channel group from rcChannelGroups.json.lz(initialized at startup)
	while (chGrpIterator != _channelGroupVector.end()){
		//PRINTLN("while called");
		chGrpPtr = *chGrpIterator;
		if (chGrpPtr->idx() == grpIdx){
			foundGrpIdx = grpIdx;
			break;
		}
		chGrpIterator++;
	};
	PRINTLN("out of while");

	if (foundGrpIdx == UNDEFINED_HEX_BYTE){
		PRINTLN("group not found");
		return retVal;//no group
	}
	else{
		PRINTLN("from channel group: ");
		PRINTLN(chGrpPtr->idx());
		PRINTLN(chGrpPtr->alias());
		PRINTLN(chGrpPtr->address());
	}

	chGrpPtr->channelVector.pop_back();
	PRINTLN("channel deleted ");
	retVal = true;

	return retVal;
}

/*
 *
 */
bool RfBridge::deleteChannelGroup(ChannelGroup::groupIdx_t grpIdx){
	bool retVal = false;
	ChannelGroup::channelGroupVector_t::iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;
	ChannelGroup::groupIdx_t foundGrpIdx = UNDEFINED_HEX_BYTE;

	//find associated channel group from rcChannelGroups.json.lz(initialized at startup)
	while (chGrpIterator != _channelGroupVector.end()){
		//PRINTLN("while called");
		chGrpPtr = *chGrpIterator;
		if (chGrpPtr->idx() == grpIdx){
			foundGrpIdx = grpIdx;
			break;
		}
		chGrpIterator++;
	};

	if (foundGrpIdx == UNDEFINED_HEX_BYTE){
		PRINTLN("group not found");
		return retVal;//no group
	}
	else {
		retVal = true;
		PRINT("group found, grpIdx: ");PRINTLN(foundGrpIdx);
	}


	//delete channels in channel group
	chGrpPtr->channelVector.clear();

	//erase requested channel group
	_channelGroupVector.erase(chGrpIterator);
	PRINTLN("channel group deleted ");

	return retVal;
}

String RfBridge::listChannelGroups()
{
	//bool retVal = false;
	DynamicJsonBuffer jsonBuffer;
	String content = "";
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	ChannelGroup* chGrpPtr = NULL;
	JsonObject& jChannelGroups = jsonBuffer.createObject();

	int i = 1;
	chGrpIterator = _channelGroupVector.begin();
	while (chGrpIterator != _channelGroupVector.end()){
		PRINT("start serializing group: ");
		chGrpPtr = *chGrpIterator; PRINTLN(i);
		content = "";
		JsonObject& jsonChGrp = jChannelGroups.createNestedObject("channel_group"+String(i));
		chGrpPtr->serialize(jsonChGrp);
		i++;
		chGrpIterator++;

		PRINTLN("group serialized");
	}
	jChannelGroups.printTo(content);
	PRINT("in RC::listChannelGroups() content: ");PRINTLN(content);
	//_storage->save("/rcChannelGroups.json", content);
	//_storage->saveLZ("/rcChannelGroupsXXX.json", content);
	//return retVal;
	return content;
}
/*
 * if given group index is undefined, all channels in memory will be listed
 * if valid group index is given, only channels within given group will be listed
 */
String RfBridge::listChannels(ChannelGroup::groupIdx_t grpIdx)
{
	//bool retVal = false;
	bool chGrpFound = false;
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	chGrpIterator = _channelGroupVector.begin();
	ChannelGroup* chGrpPtr = NULL;

	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	DynamicJsonBuffer jsonBuffer;
	JsonObject& jChannels = jsonBuffer.createObject();
	String content = "";
	int i = 1;
	while (chGrpIterator != _channelGroupVector.end()){
		PRINT("search group: ");
		chGrpPtr = *chGrpIterator;
		chIterator = chGrpPtr->channelVector.begin();
		if (grpIdx != UNDEFINED_HEX_BYTE) i = 1;
		while (chIterator != chGrpPtr->channelVector.end()){
			PRINT("start serializing channels: ");
			chPtr = *chIterator; PRINTLN(i);
			if (grpIdx == UNDEFINED_HEX_BYTE){//initialized in prototype
				JsonObject& jsonChannels = jChannels.createNestedObject("channel"+String(i));
				chPtr->serialize(jsonChannels);

			}
			else if(chGrpPtr->idx() == grpIdx){
				PRINT("found group: "); PRINTLN(grpIdx);
				JsonObject& jsonChannels = jChannels.createNestedObject("channel"+String(i));
				chPtr->serialize(jsonChannels);
				chGrpFound = true;
				//break;
			}
			i++;
			chIterator++;
		}
		if (chGrpFound == true) break;
		chGrpIterator++;
	}

	//if (chGrpFound == false) return content;//empty string

/*
	chIterator = chGrpPtr->channelVector.begin();
	while (chIterator != chGrpPtr->channelVector.end()){
		PRINT("start serializing channels: ");
		chPtr = *chIterator; PRINTLN(i);
		content = "";
		JsonObject& jsonChannels = jChannels.createNestedObject("channel"+String(i));
		chPtr->serialize(jsonChannels);
		i++;
		chIterator++;

		PRINTLN("channel serialized");
	}*/
	jChannels.printTo(content);
	PRINT("in RC::listChannels(..) content: ");PRINTLN(content);
	//_storage->save("/rcChannelGroups.json", content);
	//_storage->saveLZ("/rcChannelGroupsXXX.json", content);
	//return retVal;
	return content;
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

	int i = 1;
	chGrpIterator = _channelGroupVector.begin();
	while (chGrpIterator != _channelGroupVector.end()){
		PRINT("start serializing group: ");
		chGrpPtr = *chGrpIterator; PRINTLN(i);
		content = "";
		/*JsonArray& jsonChGrpsArray = jChannelGroupsObj.createNestedArray("channel_group"+String(i));
		JsonObject& jChannelGroup = jsonBuffer.createObject();
		chGrpPtr->serialize(jChannelGroup);
		jsonChGrpsArray.add(jChannelGroup);*/
		JsonObject& jsonChGrp = jChannelGroups.createNestedObject("channel_group" + String(i));
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
	retVal = _storage->saveLZ("/rcChannelGroups.json.lz", content);
	return retVal;
}

bool RfBridge::loadRcChannelGroups()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;

	values = _storage->openLZ("/rcChannelGroups.json.lz");
/*TODO: do I need this?:
	if (values == ""){//file not found in flash
		PRINTLN("in RC::loadRcChannelGroups() using embedded!!!!! ");
		uint osize = sizeof(RC_CHANNEL_GRP) * 1.01;
		PRINT("embedded file size: ");PRINTLN(osize);
		unsigned char *myout = new unsigned char [osize];
		unsigned char myout2[sizeof(RC_CHANNEL_GRP)];
		LZ_Uncompress( myout, myout2,2 );

	}*/
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

bool RfBridge::saveRcChannels()
{
	bool retVal = false;
	DynamicJsonBuffer jsonBuffer;
	String content = "";
	ChannelGroup::channelGroupVector_t::const_iterator chGrpIterator;
	Channel::channelVector_t::const_iterator chIterator;
	ChannelGroup* chGrpPtr = NULL;
	Channel* chPtr = NULL;
	JsonObject& jChannels = jsonBuffer.createObject();

	int i = 1;
	chGrpIterator = _channelGroupVector.begin();
	while (chGrpIterator != _channelGroupVector.end()){
		PRINT("start serializing channels in group: ");
		chGrpPtr = *chGrpIterator; PRINTLN(chGrpPtr->idx());

		//add channels to json within this group
		chIterator = chGrpPtr->channelVector.begin();
		while (chIterator != chGrpPtr->channelVector.end()){
			PRINT("serialize channel: ");
			chPtr = *chIterator;PRINTLN(i);
			JsonObject& jsonChannels = jChannels.createNestedObject("channel"+String(i));
			chPtr->serialize(jsonChannels);
			i++;
			chIterator++;
		}

		chGrpIterator++;
	}
	jChannels.printTo(content);
	PRINT("in RC::saveRcChannels() content: ");PRINTLN(content);
	retVal = _storage->save("/rcChannels.json", content);
	//_storage->saveLZ("/rcChannelsLZ.json", content);//TODO: use compression (buffer handling to be optimized)
	return retVal;
}


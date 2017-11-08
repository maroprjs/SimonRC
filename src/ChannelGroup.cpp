/*
 * ChannelGroup.cpp
 *
 *  Created on: 05.09.2017
 *      Author: maro
 */

#include "ChannelGroup.h"
#include "Debug.h"

ChannelGroup::ChannelGroup() {
	_channelGroup.alias = "";
	_channelGroupActive = false;

}

ChannelGroup::ChannelGroup(String chGrpJsonStrg){
	_channelGroup.alias = "";
	_channelGroupActive = false;
	if (deserialize(_channelGroup,chGrpJsonStrg)) PRINTLN("desrialize success!");

/*
	PRINT("idx: ");PRINTLN(String(_channel.channelIdx));
	PRINT("alias: ");PRINTLN(_channel.alias);
	PRINT("address: ");PRINTLN(_channel.address);
	PRINT("data: ");PRINTLN(_channel.data);
	PRINT("counter: ");PRINTLN(_channel.counter);
	PRINT("protocol: ");PRINTLN(_channel.protocol);
*/
}
ChannelGroup::~ChannelGroup() {
	// TODO Auto-generated destructor stub
}

bool ChannelGroup::deserialize(channelGroup_t& chGrp, String json)
{
	PRINT("in ChGrp::deserialize() incoming channel group data: "); PRINTLN(json);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    chGrp.groupIdx = root["groupIdx"];
    chGrp.alias = root["alias"].asString();
    chGrp.address = root["address"].asString();
    chGrp.btn = root["last_btn"].asString();
    chGrp.counter = (counter_t)root["counter"];
    uint8_t p = (uint8_t)root["protocol"];
    chGrp.protocol = (protocol_t)p;
    return root.success();
}


void ChannelGroup::serialize(JsonObject& root)
{
	/*
	//PRINT("in ChGrp::serialize() incoming json: "); PRINTLN(json);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["groupIdx"] = _channelGroup.groupIdx;
    root["alias"] = _channelGroup.alias;
    root["address"] = _channelGroup.address;
    root["last_btn"] = _channelGroup.btn;
    root["counter"] = _channelGroup.counter;
    root["protocol"] = (uint8_t)_channelGroup.protocol;
    //root.printTo(json);*/
    root["groupIdx"] = _channelGroup.groupIdx;
    root["alias"] = _channelGroup.alias;
    root["address"] = _channelGroup.address;
    root["last_btn"] = _channelGroup.btn;
    root["counter"] = _channelGroup.counter;
    root["protocol"] = (uint8_t)_channelGroup.protocol;
}

void ChannelGroup::activateChannelGroup(){
	PRINTLN("in ChannelGroup::activateChannelGroup() = true");
	_channelGroupActive = true;
}

void ChannelGroup::deactivate(){
	_channelGroupActive = false;
	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	chIterator = channelVector.begin();
	while (chIterator != channelVector.end()){
		chPtr = *chIterator;
		if (chPtr->active() == true){//
			chPtr->deactivate();
		}
		chIterator++;
	}
}

/*
 * Channel.cpp
 *
 *  Created on: 31.08.2017
 *      Author: pionet
 */

#include "Channel.h"
#include "Debug.h"

Channel::Channel() {
	_channel.alias = "";
	_channelActive = false;

}

Channel::Channel(String chJsonStrg){
	_channel.alias = "";
	_channelActive = false;
	if (deserialize(_channel,chJsonStrg)) PRINTLN("desrialize success!");
/*
	PRINT("idx: ");PRINTLN(String(_channel.channelIdx));
	PRINT("alias: ");PRINTLN(_channel.alias);
	PRINT("address: ");PRINTLN(_channel.address);
	PRINT("data: ");PRINTLN(_channel.data);
	PRINT("counter: ");PRINTLN(_channel.counter);
	PRINT("protocol: ");PRINTLN(_channel.protocol);
*/
}

Channel::~Channel() {
	// TODO Auto-generated destructor stub
}



bool Channel::deserialize(channel_t& ch, String json)
{
	PRINT("in Ch::deserialize() incoming channel data: "); PRINTLN(json);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    ch.channelIdx = root["chIdx"];
    ch.alias = root["alias"].asString();
    ch.address = root["address"].asString();
    ch.groupIdx = root["groupIdx"];
    return root.success();
}

void Channel::serialize(JsonObject& root)
{
    root["chIdx"] = _channel.channelIdx;
    root["alias"] = _channel.alias;
    root["address"] = _channel.address;
    root["groupIdx"] = _channel.groupIdx;
}

void Channel::activateChannel(){
	PRINT("in Channel::activateChannel() true");
	_channelActive = true;
}

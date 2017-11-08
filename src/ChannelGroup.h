/*
 * ChannelGroup.h
 *
 *  Created on: 05.09.2017
 *      Author: maro
 */

#ifndef CHANNELGROUP_H_
#define CHANNELGROUP_H_

#include <vector>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Channel.h"

class ChannelGroup {
	public:
	typedef uint16_t counter_t;
	typedef uint8_t groupIdx_t;
	enum protocol_t:uint8_t {
		SIMON,
		SOMFY,
		KOPP
	};
	struct channelGroup{
		String alias;
		groupIdx_t groupIdx;
		String address;// (in case of SIMON protocol: masterLSB+masterMSB+serial)-string characters written as HEX 0-9 & A-F
		String btn; //string characters written as HEX 0-9 & A-F
		counter_t counter;
		protocol_t protocol; //(simon, somfy, kopp)
	};
	typedef channelGroup channelGroup_t;



	typedef std::vector <ChannelGroup *> channelGroupVector_t;


public:
	ChannelGroup();
	ChannelGroup(String chGrpJsonStrg);
	virtual ~ChannelGroup();
    int counter() const {return _channelGroup.counter;}
    void counter(int c) {_channelGroup.counter = c;}
    int  counterPP() {return _channelGroup.counter++;}
    void btn(String b) {_channelGroup.btn = b;}
    String btn() {return _channelGroup.btn;}
    protocol_t protocol() const {return _channelGroup.protocol;}
    bool deserialize(channelGroup_t& chGrp, String json);
    void serialize(JsonObject& root);
    void activateChannelGroup();
    void deactivate();
    bool active() {return _channelGroupActive;};
    String address() {return _channelGroup.address;};
    String alias() {return _channelGroup.alias;};
    groupIdx_t idx() {return _channelGroup.groupIdx;};

public:
	Channel::channelVector_t channelVector;

private:
    channelGroup_t _channelGroup;
    bool _channelGroupActive;
};

#endif /* CHANNELGROUP_H_ */

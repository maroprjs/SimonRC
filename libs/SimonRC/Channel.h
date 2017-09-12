/*
 * Channel.h
 *
 *  Created on: 31.08.2017
 *      Author: pionet
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <vector>
#include <Arduino.h>
#include <ArduinoJson.h>
//#include "ChannelGroup.h"

class Channel {
	public:

	typedef uint8_t groupIdx_t;
	typedef uint8_t channelIdx_t;
	struct channel{
		String alias;//name of the remote receiver; in case of somfy this alias equals the group alias
		channelIdx_t channelIdx;
		String address;// -in case of simon 3 bytes=6hex strings (eg ABCD00), where last byte as index "00","01","02",..etc string characters written as HEX 0-9 & A-F
		groupIdx_t groupIdx;//a handle/reference to its group; each channel belongs to a group; each group has at least one channel
	};
	typedef channel channel_t;
	typedef std::vector <Channel *> channelVector_t;


public:
	Channel();
	Channel(String chJsonStrg);
	virtual ~Channel();
    int idx() const {return _channel.channelIdx;}
    void idx(int chIdx) {_channel.channelIdx = chIdx;}
    int groupIdx() const {return _channel.groupIdx;}
    void groupIdx(int groupIdx) {_channel.groupIdx = groupIdx;}
    bool deserialize(channel_t& data, String json);
    void activateChannel();
    void deactivate(){_channelActive = false;};
    bool active() {return _channelActive;};
    String address() {return _channel.address;};
    String alias() {return _channel.alias;};

private:
    channel_t _channel;
    bool _channelActive;


};

#endif /* CHANNEL_H_ */

/*
 * RfBridge.h
 *
 *  Created on: 31.08.2017
 *      Author: maro
 */

#ifndef RFBRIDGE_H_
#define RFBRIDGE_H_

#include <vector>//MRO: use this before #include <ESP8266WebServer.h>, otherwise comple error

#include "Storage.h"
#include "Transceiver.h"
#include "Channel.h"
#include "ChannelGroup.h"

//#define UNDEFINED_HEX_BYTE 0xFF ->moved to channel
//#define UNDEFINED_STRG "" ->moved to channel

class RfBridge {

public:
	RfBridge(Storage *);
	virtual ~RfBridge();
	void begin();
	void handle();
	void activateChannelGroup(ChannelGroup::groupIdx_t grpIdx,String btn_pressed);
	void activateChannel(ChannelGroup::groupIdx_t grpIdx,Channel::channelIdx_t chIdx);
	void activateTransmission();
	void createChannelGroup(ChannelGroup::protocol_t protocol, String alias = UNDEFINED_STRG, String address = UNDEFINED_STRG, ChannelGroup::counter_t cnt = 0);
	bool addChannel(ChannelGroup::groupIdx_t grpIdx, String alias = UNDEFINED_STRG, String address = UNDEFINED_STRG);
	bool deleteChannel(ChannelGroup::groupIdx_t grpIdx);//last created channel will be deleted
	bool deleteChannelGroup(ChannelGroup::groupIdx_t grpIdx);
	String listChannelGroups();
	String listChannels(ChannelGroup::groupIdx_t grpIdx = UNDEFINED_HEX_BYTE);
	bool modifyChannelGroup(ChannelGroup::groupIdx_t, ChannelGroup::protocol_t protocol = ChannelGroup::UNDEFINED, String alias = UNDEFINED_STRG, String address = UNDEFINED_STRG, ChannelGroup::counter_t cnt = 0); //mandatory ID
	//modifyChannel groupId&chId
	bool saveRcChannelGroups();
	bool saveRcChannels();
private:
	void loadChannelGroupTemplate(const char * chGrpTmplate, ChannelGroup*);
	bool loadRcConfig();
	bool loadRcChannels();
	bool loadRcChannelGroups();


private:
	Transceiver * _transceiver;
	Storage * _storage;

	ChannelGroup::channelGroupVector_t _channelGroupVector;

};

#endif /* RFBRIDGE_H_ */

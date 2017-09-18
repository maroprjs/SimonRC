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


class RfBridge {

public:
	RfBridge(Storage *);
	virtual ~RfBridge();
	void begin();
	void handle();
	void activateChannelGroup(ChannelGroup::groupIdx_t grpIdx,String btn_pressed);
	void activateChannel(ChannelGroup::groupIdx_t grpIdx,Channel::channelIdx_t chIdx);
	void commit();
private:
	bool loadRcConfig();
	bool loadRcChannels();
	bool loadRcChannelGroups();
	bool saveRcChannelGroups();

private:
	Transceiver * _transceiver;
	Storage * _storage;

	ChannelGroup::channelGroupVector_t _channelGroupVector;

};

#endif /* RFBRIDGE_H_ */

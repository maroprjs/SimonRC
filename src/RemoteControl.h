/*
 * RemoteControl.h
 *
 *  Created on: 31.08.2017
 *      Author: maro
 */

#ifndef REMOTECONTROL_H_
#define REMOTECONTROL_H_


#include "MaroWebServer.h"
#include "Transceiver.h"
#include "Channel.h"
#include "ChannelGroup.h"


class RemoteControl {

public:
	RemoteControl(MaroWebServer *);
	virtual ~RemoteControl();
	void begin();
	void handle();

private:
	void handleWebGuiHttp();
	bool loadRcConfig();
	bool loadRcChannels();
	bool loadRcChannelGroups();
	bool saveRcChannelGroups();

private:
	MaroWebServer * _myWebServer;
	Transceiver * _transceiver;

	ChannelGroup::channelGroupVector_t _channelGroupVector;

};

#endif /* REMOTECONTROL_H_ */

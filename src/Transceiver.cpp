/*
 * Transceiver.cpp
 *
 *  Created on: 31.08.2017
 *      Author: maro
 */

#include "Transceiver.h"
#include "Debug.h"
Transceiver::Transceiver() {
	_cc1101 = new CC1101();
	simon = new JL();
	state = IDLE;

}

void Transceiver::begin(){
	//TODO: fill when needed
	pinMode(TX_PIN, OUTPUT);
	//if C1101 attached:TODO: take different hw into account!
	beginCC1101();//in order not to modify the c1101 library


}

void Transceiver::handle(){
	switch (state){
	case IDLE:
		break;
	case TRANSMITTING:
		transmit();
		break;
	case RECEIVING:
		break;
	}
}

Transceiver::~Transceiver() {
	// TODO Auto-generated destructor stub
}



//keeps record of channels and channel groups to prepare for sending; will be cleared/deactivated after transmit
void Transceiver::registerChannelGroups(ChannelGroup::channelGroupVector_t chGrpV){
	PRINTLN("Transceiver::registerChannelGroups called");
	_channelGroupVector = chGrpV;
}

void Transceiver::transmit(){
	PRINTLN("in Transceiver::transmit()");
	//find active channelGroups and channels within those groups to be prepared for transmission:
	ChannelGroup::channelGroupVector_t::const_iterator chGroupIterator;
	ChannelGroup* chGrpPtr = NULL;
	chGroupIterator = _channelGroupVector.begin();
	while (chGroupIterator != _channelGroupVector.end()){
		chGrpPtr = *chGroupIterator;
		if (chGrpPtr->active()){
			PRINT("in Transceiver::transmit() found active Group: ");PRINTLN(chGrpPtr->alias());
			//TODO:chose sender type according protocol in group
			simon->encode(chGrpPtr);
			simon->stream(_cc1101);
			chGrpPtr->deactivate();//for next sending
			this->state = IDLE;
		}
		chGroupIterator++;

	}
}


void Transceiver::beginCC1101(){
	_cc1101->init();
	_cc1101->setSyncWord(0xC7,false);
	_cc1101->setCarrierFreq(CFREQ_433);
	_cc1101->disableAddressCheck();
	PRINT("CC1101_PARTNUM: "); //cc1101=0
	PRINTLN(_cc1101->readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
	PRINT("CC1101_VERSION: "); //cc1101=4
	PRINTLN(_cc1101->readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
	PRINT("CC1101_MARCSTATE: ");
	PRINTLN(_cc1101->readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
	PRINT("TX-POWER: ");
	PRINTLN(_cc1101->readReg(CC1101_FREND0, CC1101_CONFIG_REGISTER));

}



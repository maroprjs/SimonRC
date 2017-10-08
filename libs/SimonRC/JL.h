/*
 * JL.h
 *
 *  Created on: 06.09.2017
 *      Author: maro
 *
 */

#ifndef JL_H_
#define JL_H_
#include "ChannelGroup.h"
#include <cc1101.h>
#include "Arduino.h"

class JL {

#define Lowpulse         400                //Defines Pulse-Width in us. Adapt for your use...
#define Highpulse        800
#define RC_CHANNEL_GRP_TEMPLATE_SIZE_LZ 155
#define RC_CHANNEL_GRP_TEMPLATE_SIZE 184
#define MAX_NUM_OF_CHANNELS_IN_GRP 0xF

	#define BUTTON_DOWN "down"
	#define BUTTON_STOP "stop"
	#define BUTTON_UP "up"
	#define BUTTON_UP_DOWN "updown"
	#define BUTTON_LEARN "learn"
	#define BUTTON_ERASE "erase"
	#define BUTTON_COPY "copy"
private:
	const char RC_CHANNEL_GRP_TEMPLATE[RC_CHANNEL_GRP_TEMPLATE_SIZE_LZ] = {
	    0x00, 0x7b, 0x0d, 0x0a, 0x09, 0x22, 0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65,
	    0x6c, 0x5f, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x22, 0x3a, 0x20, 0x00, 0x04,
	    0x15, 0x09, 0x22, 0x00, 0x05, 0x0e, 0x49, 0x64, 0x78, 0x22, 0x3a, 0x20,
	    0x30, 0x2c, 0x00, 0x05, 0x12, 0x61, 0x6c, 0x69, 0x61, 0x73, 0x22, 0x3a,
	    0x20, 0x22, 0x6a, 0x61, 0x72, 0x6f, 0x6c, 0x69, 0x66, 0x74, 0x20, 0x72,
	    0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x72, 0x20, 0x00, 0x05, 0x2d, 0x20,
	    0x22, 0x00, 0x07, 0x28, 0x64, 0x64, 0x72, 0x65, 0x73, 0x00, 0x05, 0x2a,
	    0x32, 0x37, 0x31, 0x39, 0x33, 0x41, 0x39, 0x42, 0x31, 0x31, 0x37, 0x43,
	    0x30, 0x38, 0x33, 0x35, 0x41, 0x42, 0x42, 0x41, 0x30, 0x36, 0x00, 0x07,
	    0x28, 0x6c, 0x61, 0x73, 0x74, 0x5f, 0x62, 0x74, 0x6e, 0x00, 0x04, 0x29,
	    0x73, 0x74, 0x6f, 0x70, 0x00, 0x07, 0x17, 0x63, 0x6f, 0x75, 0x6e, 0x74,
	    0x65, 0x72, 0x00, 0x0a, 0x78, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f,
	    0x6c, 0x00, 0x04, 0x12, 0x0d, 0x0a, 0x09, 0x7d, 0x0d, 0x0a, 0x7d
	};


	struct btn{
		String btn;
		uint8_t btnCode(){
			   if ( btn == "down" ) {return 0x2;}
			    else if ( btn == "stop" ) {return 0x4;}
			    else if ( btn == "up" ) {return 0x8;}
			    else if ( btn == "updown" ) {return 0xA; }
			    else if ( btn == "learn" ) {return 0xA; }//followed by 1xstop
			    else if ( btn == "erase" ) {return 0xA; }//followed by 6xstop and 1xup
			    else if ( btn == "copy" ) {return 0xA; }//followed by 8xstop
			    else  {return 0x4;};
		};
	};
	typedef btn btn_t;
	typedef unsigned long key_t;
	typedef uint32_t serialN_t;

	enum jlMode_t:uint8_t {//TODO: check, is this needed???
		SINGLE,
		MULTI
	};

	struct discriminator{ //disc: MSB="index mask", MSB-1="index int value",LSBs = "16bit counter" -> =4bytes
		key_t chMaskIdx_low;
		key_t chMaskIdx_high;
		jlMode_t jlMode;
		ChannelGroup::groupIdx_t grpIdx;//when only one channel in group grpIdx = chIdx
		serialN_t serialN;
		ChannelGroup::counter_t counter;
		key_t part1(){//mask of first 8 receiver in group
			key_t t = chMaskIdx_low << 8;//prepare the two MSBs
			t = t | grpIdx;
			t = t << 16;//add the two LSB's
			return t | counter;
		};
		key_t part2(){//mask of the next 8 receiver in group
			return chMaskIdx_high;
		};
	};
	typedef discriminator discriminator_t;


public:
	JL();
	virtual ~JL();
	void loadChannelGroupTemplate(ChannelGroup* chGrpPtr,ChannelGroup::groupIdx_t);
	bool loadChannelTemplate(ChannelGroup* chGrpPtr, Channel* chPtr, Channel::channelIdx_t);
	void encode(ChannelGroup*);
	void send(CC1101*);
public:
	//discriminator_t discriminator;
private:
	void generateDeviceKey(ChannelGroup* chGrp,discriminator_t* disc);
	void generateHopCode(discriminator_t* disc);
	//key_t getChannelIndexMask(ChannelGroup* chGrp);
	//void calculateDiscriminator(ChannelGroup* chGrp, discriminator_t&);
	void generateDiscriminator(ChannelGroup* chGrp, discriminator_t&);
	key_t getSeedLS(String);
	key_t getSeedMS(String);
	String getSeedAsString(String iStrg);
	key_t getGroupSerial(String);
	key_t getSerial(String);
	key_t getSerialIdx(String);
	String getSerialIdxAsString(String iStrg);
	key_t hex2int(char *hex);
	bool buttonUp();
	bool buttonStop();
	bool buttonDown();
	bool buttonUpDown();
	bool buttonLearn();
	bool buttonErase();
	bool buttonCopy();
	void updateDiscriminator(String button);
	void entertx();
	void enterrx();
	void senden(uint8_t);
	void group_h();
	void frame(int l);
	void delayMicros(uint32_t d);
	void myprint(uint64_t value);
private:
	ChannelGroup* _chGrp;
	discriminator_t _discriminator;
	key_t _deviceKeyLow;
	key_t _deviceKeyHigh;
	key_t _hopCode;


	uint32_t _rx_time;
	CC1101 * _cc1101;
	btn_t _btnPressed;

};

#endif /* JL_H_ */

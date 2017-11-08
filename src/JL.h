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

	#define BUTTON_DOWN "down"
	#define BUTTON_STOP "stop"
	#define BUTTON_UP "up"
	#define BUTTON_UP_DOWN "updown"
	#define BUTTON_LEARN "learn"
	#define BUTTON_ERASE "erase"
	#define BUTTON_COPY "copy"

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
		key_t part1(){
			key_t t = chMaskIdx_low << 8;//prepare the two MSBs
			t = t | grpIdx;
			t = t << 16;//add the two LSB's
			return t | counter;
		};
		key_t part2(){
			return chMaskIdx_high;
		};
	};
	typedef discriminator discriminator_t;


public:
	JL();
	virtual ~JL();
	void encode(ChannelGroup*);
	void send(CC1101*);
public:
	//discriminator_t discriminator;
private:
	void generateDeviceKey(ChannelGroup* chGrp,discriminator_t* disc);
	void generateHopCode(ChannelGroup* chGrp,discriminator_t* disc);
	//key_t getChannelIndexMask(ChannelGroup* chGrp);
	//void calculateDiscriminator(ChannelGroup* chGrp, discriminator_t&);
	void generateDiscriminator(ChannelGroup* chGrp, discriminator_t&);
	key_t getKeyLow(String);
	key_t getKeyHigh(String);
	key_t getSerial(String);
	key_t getSerialIdx(String);
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

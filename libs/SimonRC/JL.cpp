/*
 * JL.cpp
 *
 *  Created on: 06.09.2017
 *      Author: maro
 */

#include "JL.h"
#include "Keeloq.h"
#include "lz.h"
#include "Debug.h"

JL::JL() {
	_deviceKeyLow = 0;
	_deviceKeyHigh = 0;
	_hopCode = 0;
	_cc1101 = NULL;
	_rx_time = 0;
	_chGrp = NULL;

}

JL::~JL() {
	// TODO Auto-generated destructor stub
}

void JL::loadChannelGroupTemplate(ChannelGroup* chGrpPtr,ChannelGroup::groupIdx_t newIdx){
	//newIdx = 3;//TODO: delete this!!!!
	DynamicJsonBuffer jsonBuffer;
	unsigned char *chGrpTemplate = new unsigned char [RC_CHANNEL_GRP_TEMPLATE_SIZE];
	String outStrg = "";

	PRINTLN("in JL::loadChannelGroupTemplate(..)");

	LZ_Uncompress( (unsigned char*)RC_CHANNEL_GRP_TEMPLATE, chGrpTemplate,RC_CHANNEL_GRP_TEMPLATE_SIZE_LZ);

	for ( int i = 0; i < RC_CHANNEL_GRP_TEMPLATE_SIZE; i++ ) outStrg = outStrg + (char)chGrpTemplate[i];

	delete chGrpTemplate;

	JsonObject& rcChannelGroupJson = jsonBuffer.parseObject(outStrg);
	if (rcChannelGroupJson.success()){
		//chGrpPtr->idx(newIdx);

		String addrTmplte = rcChannelGroupJson["channel_group"]["address"].asString();
		String addr = getSeedAsString(addrTmplte) + String(getGroupSerial(addrTmplte) + newIdx,HEX) + getSerialIdxAsString(addrTmplte);
		addr.toUpperCase();
		chGrpPtr->address(addr);
		PRINTLN(chGrpPtr->address());

		String aliasTmplte = rcChannelGroupJson["channel_group"]["alias"].asString();
		aliasTmplte = aliasTmplte + " " + String(newIdx);
		chGrpPtr->alias(aliasTmplte);
	}

	//PRINTLN(outStrg);
}

bool JL::loadChannelTemplate(ChannelGroup* chGrpPtr, Channel* chPtr,Channel::channelIdx_t newIdx){
	bool retVal = false;

	if (newIdx > MAX_NUM_OF_CHANNELS_IN_GRP) return retVal;
	else retVal = true;

	String addrTmplte = chGrpPtr->address();
	String addr = getSeedAsString(addrTmplte) + String(getGroupSerial(addrTmplte),HEX) + "0" + String(newIdx,HEX);
	addr.toUpperCase();
	chPtr->address(addr);
	PRINTLN(chPtr->address());

	String aliasTmplte = "simon channel " + String(newIdx + 1);
	chPtr->alias(aliasTmplte);

	chPtr->groupIdx(chGrpPtr->idx());

	return retVal;
}

void JL::encode(ChannelGroup* chGrp){
	_chGrp = chGrp;
	_btnPressed.btn = chGrp->btn();

	generateDiscriminator(chGrp,_discriminator);	//
	PRINTLN("Discriminatro calculated: ");
	PRINT("idxmask high: ");PRINTLNH(_discriminator.chMaskIdx_high);
	PRINT("idxmask low: ");PRINTLNH(_discriminator.chMaskIdx_low);
	PRINT("counter: ");PRINTLN(_discriminator.counter);
	PRINT("idx: ");PRINTLN(_discriminator.grpIdx);
	PRINT("jlMode: ");PRINTLN(_discriminator.jlMode);
	PRINT("serial: ");PRINTLNH(_discriminator.serialN);
	PRINT("part1: ");PRINTLNH(_discriminator.part1());
	PRINT("part2: ");PRINTLNH(_discriminator.part2());
	generateDeviceKey(chGrp,&_discriminator); 	//input needed: group adress, serial dpendng on mode
								//output device key
	generateHopCode(&_discriminator);//input needed: device key, discriminator
	//generateTxStream(chGrp);???
}



void JL::generateDeviceKey(ChannelGroup* chGrp, discriminator_t* disc){
	 Keeloq klq(getSeedMS(chGrp->address()), getSeedLS(chGrp->address()));
	 String address = "";
	 PRINT("in JL::generateDeviceKey(..) serialN: ");PRINTLNH(disc->serialN);
	 key_t key = disc->serialN | 0x20000000;
	 _deviceKeyLow = klq.decrypt(key);
	 key = disc->serialN | 0x60000000;
	 _deviceKeyHigh = klq.decrypt(key);
	 PRINT("Device Key: ");PRINTH(_deviceKeyHigh);PRINTLNH(_deviceKeyLow);
}


void JL::generateHopCode(discriminator_t* disc){
	Keeloq klq(_deviceKeyHigh, _deviceKeyLow);
	//calculateDiscriminator(chGrp, discriminator);
	PRINT("in JL::generateHopCode calculated discr.: ");PRINTH(disc->part1());PRINT(" ");PRINTLNH(disc->part2());
	_hopCode = klq.encrypt(disc->part1());
	PRINT("in JL::generateHopCode calculated hopCode.: ");PRINTLNH(_hopCode);
}


/*
JL::key_t JL::getChannelIndexMask(ChannelGroup* chGrp){
	key_t retVal = 0;
	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	key_t mask = 0x1;
	uint8_t chCount = 0;
	uint8_t activeChannelIdx = 0;
	while (chIterator != chGrp->channelVector.end()){
		chPtr = *chIterator;
		if ((chPtr->active() == true) && (chPtr->idx() < 8)){//
			chCount++;
			activeChannelIdx = getSerialIdx(chPtr->address());
			mask = mask << activeChannelIdx;
			retVal = retVal | mask;
			mask = 0x1;
		}
		chIterator++;
	}
	if (chCount == 1){//single activity
		retVal = retVal << 8;
		retVal = retVal | activeChannelIdx;
	}else {//group activity
		retVal = retVal << 8;
		retVal = retVal | getSerialIdx(chGrp->address());
	}

	PRINT("in JL::getChannelIndexMask(..), output: "); PRINTLNH(retVal);
	return retVal;
}

JL::key_t JL::getChannelIndexMask(ChannelGroup* chGrp){
	key_t retVal = 0;
	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	key_t mask = 0x1;
	_activeChannelCount = 0;
	_activeChannelIdx = 0;
	while (chIterator != chGrp->channelVector.end()){
		_activeChannelCount++;
		chPtr = *chIterator;
		if (chPtr->active()){//
			_activeChannelIdx = getSerialIdx(chPtr->address());
			mask = mask << _activeChannelIdx;
			retVal = retVal | mask;
			mask = 0x1;
		}
		chIterator++;
	}
	PRINT("in JL::getChannelIndexMask(..), output: "); PRINTLNH(retVal);
	return retVal;
}

void JL::calculateDiscriminator(ChannelGroup* chGrp, discriminator_t& disc){
	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	key_t chIdxMask = 0x0;
	key_t mask = 0x1;
	uint8_t chCount = 0;
	uint8_t activeChannelIdx = 0;
	serialN_t serialN = 0;
	//store counter element of discriminator:
	disc.counter = chGrp->counterPP();
	//create channel index mask
	chIterator = chGrp->channelVector.begin();
	while (chIterator != chGrp->channelVector.end()){
		chPtr = *chIterator;
		if ((chPtr->active() == true) && (chPtr->idx() < 8)){//
			chCount++;
			activeChannelIdx = getSerialIdx(chPtr->address());
			serialN = getSerial(chPtr->address());//needed for single mode
			mask = mask << activeChannelIdx;
			chIdxMask = chIdxMask | mask;
			mask = 0x1;
		}
		chIterator++;
	}
	//distinguish first 8 channels in bit array:
	disc.chMaskIdx_low = mask & 0x000000FF;
	//distinguis the other 8 channels in bit array:
	disc.chMaskIdx_high = mask & 0x0000FF00;
	//chose apropriate channel index:
	if (chCount == 1){//single activity; take channel index as distinguisher
		disc.grpIdx = activeChannelIdx;
		disc.serialN = serialN;
	}else {//group activity; take group index as distinguisher
		disc.grpIdx = getSerialIdx(chGrp->address());
		disc.serialN = getSerial(chGrp->address());

	}

}
*/

void JL::generateDiscriminator(ChannelGroup* chGrp, discriminator_t& disc){
	Channel::channelVector_t::const_iterator chIterator;
	Channel* chPtr = NULL;
	key_t chIdxMask = 0x0;
	key_t mask = 0x1;
	uint8_t chCount = 0;
	uint8_t activeChannelIdx = 0;
	serialN_t serialN = 0;
	//store counter element of discriminator:
	disc.counter = chGrp->counterPP();
	//create channel index mask
	chIterator = chGrp->channelVector.begin();
	while (chIterator != chGrp->channelVector.end()){
		chPtr = *chIterator;
		PRINTLN(chPtr->alias());
		PRINTLN(chPtr->address());
		if (chPtr->active() == true){//
			chCount++;
			activeChannelIdx = getSerialIdx(chPtr->address());
			PRINT("active ch idx: ");PRINTLN(activeChannelIdx);
			serialN = getSerial(chPtr->address());//needed for single mode
			PRINT("active ch serial: ");PRINTLN(serialN);
			mask = mask << activeChannelIdx;
			PRINT("active channel mask 1st: ");PRINTLN(mask);
			chIdxMask = chIdxMask | mask;
			PRINT("active channel mask 2st: ");PRINTLN(chIdxMask);
			mask = 0x1;
		}
		chIterator++;
	}
	//distinguish first 8 channels in bit array:
	disc.chMaskIdx_low = chIdxMask & 0x000000FF;
	//distinguis the other 8 channels in bit array:
	disc.chMaskIdx_high = chIdxMask & 0x0000FF00;
	disc.chMaskIdx_high = chIdxMask >> 8;
	//chose apropriate channel index:
	if (chCount == 1){//single activity; take channel index as distinguisher
		disc.grpIdx = activeChannelIdx;
		disc.serialN = serialN;
		disc.jlMode = SINGLE;
	}else {//group activity; take group index as distinguisher
		disc.grpIdx = getSerialIdx(chGrp->address());
		disc.serialN = getSerial(chGrp->address());
		disc.jlMode = MULTI;

	}

}

//btn_map_t btn;
//btn.strgToBtnCode("stop");

//"FEDCBA9876543210ABCD01" to 0x76543210
JL::key_t JL::getSeedLS(String iStrg){
	PRINT("in JL::getSeedLS(..), input: "); PRINTLN(iStrg);
	const uint8_t SIZE_OF_LOW_KEY = 8;// in [nibbles]
	const uint8_t BEGIN_OFFSET_LOW_KEY = 8;// in [nibbles]
	key_t retVal = 0;
	String ss = iStrg.substring(BEGIN_OFFSET_LOW_KEY, BEGIN_OFFSET_LOW_KEY + SIZE_OF_LOW_KEY);
	PRINT("in JL::getSeedLS(..), output String: "); PRINTLN(ss);
	retVal = hex2int((char*)ss.c_str());
	PRINT("in JL::getSeedLS(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

//"FEDCBA9876543210ABCD01" to 0xFEDCBA98
JL::key_t JL::getSeedMS(String iStrg){
	PRINT("in JL::getSeedMS(..), input: "); PRINTLN(iStrg);
	const uint8_t SIZE_OF_HIGH_KEY = 8;// in [nibbles]
	const uint8_t BEGIN_OFFSET_HIGH_KEY = 0;// in [nibbles]
	key_t retVal = 0;
	String ss = iStrg.substring(BEGIN_OFFSET_HIGH_KEY, BEGIN_OFFSET_HIGH_KEY + SIZE_OF_HIGH_KEY);
	PRINT("in JL::getSeedMS(..), output String: "); PRINTLN(ss);
	retVal = hex2int((char*)ss.c_str());
	PRINT("in JL::getSeedMS(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

//"FEDCBA9876543210ABCD01" to FEDCBA9876543210
String JL::getSeedAsString(String iStrg){
	PRINT("in JL::getSeedAsString(..), input: "); PRINTLN(iStrg);
	const uint8_t SIZE_OF_SEED = 16;// in [nibbles]
	const uint8_t BEGIN_OFFSET_SEED = 0;// in [nibbles]
	String retVal = "";
	String ss = iStrg.substring(BEGIN_OFFSET_SEED, BEGIN_OFFSET_SEED + SIZE_OF_SEED);
	PRINT("in JL::getSeedAsString(..), output String: "); PRINTLN(ss);
	//retVal = hex2int((char*)ss.c_str());
	retVal = ss;
	//PRINT("in JL::getSeedAsString(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

//"FEDCBA9876543210ABCD01" to 0xABCD
JL::key_t JL::getGroupSerial(String iStrg){
	PRINT("in JL::getGroupSerial(..), input: "); PRINTLN(iStrg);
	const uint8_t SIZE_OF_GRP_SERIAL = 4;// in [nibbles]
	const uint8_t BEGIN_OFFSET_GRP_SERIAL = 16;// in [nibbles]
	key_t retVal = 0;
	String ss = iStrg.substring(BEGIN_OFFSET_GRP_SERIAL, BEGIN_OFFSET_GRP_SERIAL + SIZE_OF_GRP_SERIAL);
	PRINT("in JL::getGroupSerial(..), output String: "); PRINTLN(ss);
	retVal = hex2int((char*)ss.c_str());
	PRINT("in JL::getGroupSerial(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

//"FEDCBA9876543210ABCD01" to 0xABCDE01
JL::key_t JL::getSerial(String iStrg){
	PRINT("in JL::getSerial(..), input: "); PRINTLN(iStrg);
	const uint8_t BEGIN_OFFSET_SERIAL = 16;// in [nibbles]
	key_t retVal = 0;
	iStrg.remove(0, BEGIN_OFFSET_SERIAL);
	PRINT("in JL::getSerial(..), output String: "); PRINTLN(iStrg);
	retVal = hex2int((char*)iStrg.c_str());
	PRINT("in JL::getSerial(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}



//"FEDCBA9876543210ABCD01" to 0x01
JL::key_t JL::getSerialIdx(String iStrg){
	PRINT("in JL::getSerialIdx(..), input: "); PRINTLN(iStrg);
	const uint8_t BEGIN_OFFSET_SERIAL = 20;// in [nibbles]
	key_t retVal = 0;
	iStrg.remove(0, BEGIN_OFFSET_SERIAL);
	PRINT("in JL::getSerialIdx(..), output String: "); PRINTLN(iStrg);
	retVal = hex2int((char*)iStrg.c_str());
	PRINT("in JL::getSerialIdx(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

//"FEDCBA9876543210ABCD01" to "01"
String JL::getSerialIdxAsString(String iStrg){
	PRINT("in JL::getSerialIdxAsString(..), input: "); PRINTLN(iStrg);
	const uint8_t BEGIN_OFFSET_SERIAL = 20;// in [nibbles]
	String retVal = "";
	iStrg.remove(0, BEGIN_OFFSET_SERIAL);
	PRINT("in JL::getSerialIdxAsString(..), output String: "); PRINTLN(iStrg);
	//retVal = hex2int((char*)iStrg.c_str());
	retVal = iStrg;
	//PRINT("in JL::getSerialIdx(..), output Hex: "); PRINTLNH(retVal);
	return retVal;
}

/**
 * https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
 *
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
JL::key_t JL::hex2int(char *hex) {
    key_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

void JL::enterrx() {
  _cc1101->setRxState();
  int marcState;
  _rx_time = micros();
  while (((marcState = _cc1101->readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x0D )
  {
    if (micros() - _rx_time > 2000) break; //Quit when marcState does not change...
  }
}

void JL::entertx() {
  _cc1101->setTxState();
  int marcState;
  _rx_time = micros();
  while (((marcState = _cc1101->readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x13)
  {
    if (micros() - _rx_time > 2000) break; //Quit when marcState does not change...
  }

}

void JL::send(CC1101* cc1101){
	_cc1101 = cc1101;
	if (buttonUp()) return;
	if (buttonStop()) return;
	if (buttonDown()) return;
	if (buttonUpDown()) return;
	if (buttonLearn()) return;
	if (buttonErase()) return;
	if (buttonCopy()) return;
}

bool JL::buttonUp(){
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_UP) return false;
	PRINTLN("JL::buttonUp() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
	return retVal;
}

bool JL::buttonStop(){
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_STOP) return false;
	PRINTLN("JL::buttonStop() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
	return retVal;
}

bool JL::buttonDown(){
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_DOWN) return false;
	PRINTLN("JL::buttonDown() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
	return retVal;
}

bool JL::buttonUpDown(){
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_UP_DOWN) return false;
	PRINTLN("JL::buttonUpDown() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
	return retVal;
}

bool JL::buttonLearn(){//up & down, followed by 1xstop
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_LEARN) return false;
	PRINTLN("JL::buttonLearn() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
    delay(1000);
    updateDiscriminator(BUTTON_STOP);
	generateHopCode(&_discriminator);
    _cc1101->setTxState();  //
    senden(_btnPressed.btnCode());             //
    _cc1101->setIdleState();
	return retVal;
}



bool JL::buttonErase(){//up & down, followed by 6xstop and 1xup
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_ERASE) return false;
	PRINTLN("JL::buttonErase() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
    _cc1101->setTxState();  //
    for (int i = 0; i<6;i++){//six times stop
    	delay(800);
    	updateDiscriminator(BUTTON_STOP);
    	generateHopCode(&_discriminator);
    	senden(_btnPressed.btnCode());
    }
    delay(1000);
    updateDiscriminator(BUTTON_UP);         //one time up
    generateHopCode(&_discriminator);
    senden(_btnPressed.btnCode());
    _cc1101->setIdleState();//


	return retVal;
}

bool JL::buttonCopy(){//up & down, followed by 6xstop and 1xup
	bool retVal = true;
	if (_btnPressed.btn != BUTTON_COPY) return false;
	PRINTLN("JL::buttonCopy() called");
	entertx();
	senden(_btnPressed.btnCode());
	enterrx();
    _cc1101->setTxState();  //
    for (int i = 0; i<8;i++){//eight times stop
    	delay(800);
    	updateDiscriminator(BUTTON_STOP);
    	generateHopCode(&_discriminator);
    	senden(_btnPressed.btnCode());
    }
    _cc1101->setIdleState();//


	return retVal;
}

void JL::updateDiscriminator(String button){
	_chGrp->btn(button);
	_btnPressed.btn = _chGrp->btn();
	_discriminator.counter = _chGrp->counterPP();
}


//Simple TX routine.
//Send code two times. In case of one shutter did not "hear" the command.
void JL::senden(uint8_t btnCode) {
  //entertx();
	uint64_t data = btnCode;
	uint64_t pack = 0;
	pack = data << 60;
	data = _discriminator.serialN;
	data = data << 32;
	pack = pack | data;
	data = _hopCode;
	pack = pack | data;
	myprint(pack);
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(4, LOW);//CC1101 in TX Mode+
    delayMicroseconds(1150);
    frame(10);
    delayMicroseconds(3500);//Waittime
    for (int i = 0; i < 64; i++) {
      int out = ((pack >> i) & 0x1);//Bitmask to get MSB and send it first
      if (out == 0x1)
      {
        digitalWrite(4, LOW);//Simple encoding of Bit state 1
        delayMicroseconds(Lowpulse);
        digitalWrite(4, HIGH);
        delayMicroseconds(Highpulse);
      }
      else
      {
        digitalWrite(4, LOW);//Simple encoding of Bit state 0
        delayMicroseconds(Highpulse);
        digitalWrite(4, HIGH);
        delayMicroseconds(Lowpulse);
      }
    }
    group_h(); //Last 8Bit. For motor 8-16.
    delayMicroseconds(2500);

  }

}


//Sending of high_group_bits 8-16
void JL::group_h() {
  for (int i = 0; i < 8; i++) {

    int out = ((_discriminator.part2() >> i) & 0x1);//Bitmask to get MSB and send it first
    if (out == 0x1)
    {
      digitalWrite(4, LOW);//Simple encoding of Bit state 1
      delayMicroseconds(Lowpulse);
      digitalWrite(4, HIGH);
      delayMicroseconds(Highpulse);
    }
    else
    {
      digitalWrite(4, LOW);//Simple encoding of Bit state 0
      delayMicroseconds(Highpulse);
      digitalWrite(4, HIGH);
      delayMicroseconds(Lowpulse);
    }
  }
}

//Generates sync-pulses

void JL::frame(int l) {
  for (int i = 0; i < l; ++i) {
    digitalWrite(4, LOW);
    delayMicroseconds(Highpulse);
    digitalWrite(4, HIGH);
    delayMicroseconds(Lowpulse);
  }
}

void JL::delayMicros(uint32_t d) {
#ifdef SYS_DELAY
  uint32_t t = micros() + d;
  while(micros() < t);
#else
  delayMicroseconds(d);
#endif
}

//https://forum.arduino.cc/index.php?topic=378359.0
//#include <math.h>
void JL::myprint(uint64_t value)
{
    const int NUM_DIGITS    = log10(value) + 1;

    char sz[NUM_DIGITS + 1];

    sz[NUM_DIGITS] =  0;
    for ( size_t i = NUM_DIGITS; i--; value /= 10)
    {
        sz[i] = '0' + (value % 10);
    }

    Serial.print(sz);
}

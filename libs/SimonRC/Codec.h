/*
 * Codec.h
 *
 *  Created on: 31.08.2017
 *      Author: pionet
 */

#ifndef CODEC_H_
#define CODEC_H_

#include <Arduino.h>
#include "Keeloq.h"
//TODO: #include "Somfy.h"
//TODO: #include "Kopp.h"

class Codec {
public:
    enum algorithm_t:uint8_t {
      KEELOQ_SIMON,
      SOMFY,
      KOPP
    };
public:
	Codec();
	virtual ~Codec();
	void begin();
	void handle();

};

#endif /* CODEC_H_ */

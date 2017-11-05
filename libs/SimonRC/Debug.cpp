/*
 * Debug.cpp
 *
 *  Created on: 10.05.2017
 *      Author: maro
 */


#include <Arduino.h>
#include "Debug.h"
#ifdef DEBUG //TODO: get this out of here:
	//format bytes
	String formatBytes(size_t bytes){
		if (bytes < 1024){
			return String(bytes)+"B";
		} else if(bytes < (1024 * 1024)){
			return String(bytes/1024.0)+"KB";
		} else if(bytes < (1024 * 1024 * 1024)){
	   return String(bytes/1024.0/1024.0)+"MB";
		} else {
			return String(bytes/1024.0/1024.0/1024.0)+"GB";
	 }
	}

	//https://forum.arduino.cc/index.php?topic=378359.0
	//#include <math.h>
	void myprint(uint64_t value) {
	    const int NUM_DIGITS    = log10(value) + 1;
	    char sz[NUM_DIGITS + 1];
	    sz[NUM_DIGITS] =  0;
	    for ( size_t i = NUM_DIGITS; i--; value /= 10)
	    {
	        sz[i] = '0' + (value % 10);
	    }
	    PRINT(sz);
	};

#endif

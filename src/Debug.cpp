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

#endif

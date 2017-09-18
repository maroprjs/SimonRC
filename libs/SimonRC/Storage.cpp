/*
 * Storage.cpp
 *
 *  Created on: 19.05.2017
 *      Author: maro
 */

#include "Storage.h"
#include "Debug.h"
#include "lz.h"

Storage::Storage() {
	// TODO Auto-generated constructor stub

}

Storage::~Storage() {
	// TODO Auto-generated destructor stub
}

void Storage::begin(){
	bool result = SPIFFS.begin();
	PRINTLN("SPIFFS opened: " + result);

}

String Storage::open(String name){
	String retVal = "";

	fs::File f = SPIFFS.open(name, "r");
	if (!f) {
		PRINT(name);PRINTLN(" not set/found!");
		retVal = "";
	}
	else {  //file exists;
		//retVal = f.readStringUntil('\n');  //read json
		retVal = f.readString();  //read json
		f.close();
	}
	return retVal;
}

String Storage::openLZ(String name){
	String retVal = "";

	fs::File f = SPIFFS.open(name, "r");
	if (!f) {
		PRINT(name);PRINTLN(" not set/found!");
		retVal = "";
	}
	else {  //file exists;
		//retVal = f.readStringUntil('\n');  //read json
		retVal = f.readString();  //read json
		f.close();
		//retVal =
	};
	return retVal;
}

bool Storage::save(String fname, String content){
	bool retVal = false;
	fs::File file = SPIFFS.open(fname, "w");
	if (file)
	{
		file.println(content);  //save json data
		file.close();
		retVal = true;
	}

	return retVal;
}

bool Storage::saveLZ(String fname, String content){
	bool retVal = false;
	fs::File file = SPIFFS.open(fname, "w");
	if (file)
	{
		char *buf = new char [2000];//[1926];
		char *obuf = new char [2000];//[1926]
		content.toCharArray(buf, 2000);
		int ret = LZ_Compress( (unsigned char*)buf, (unsigned char*)obuf,2000);
		PRINTLN(ret);
		String socc = "";
		  for (int i = 0; i < ret; i++ ){
			  //char * o = (char*)obuf;
			  //Serial.print(String(o[i]));
			  String so = String(buf[i]);
			  //Serial.print(myout[i]);
			  socc = socc + so;

		  }
		//file.println(content);  //save json data
		  PRINT(socc);
		file.println(obuf);
		file.close();
		retVal = true;
	}

	return retVal;
}

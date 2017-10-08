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
//		retVal = f.readString();  //read json
		uint16_t fsize = f.size();
		char *ibuf = new char [fsize * 4];
		char *obuf = new char [fsize * 4];
		f.readBytes(ibuf, fsize);
		f.close();
		LZ_Uncompress( (unsigned char*)ibuf, (unsigned char*)obuf,fsize );
		//PRINT("read return: ");PRINTLN(fsize);
		//PRINTLN(String(obuf));
		retVal = String(obuf);
		delete ibuf;
		delete obuf;
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

//TODO: change buffer handling
bool Storage::saveLZ(String fname, String content){
	bool retVal = false;
	fs::File file = SPIFFS.open(fname, "w");
	if (file)
	{
		PRINT("content size: "); PRINTLN(content.length());
		PRINTLN(sizeof(content));
		content.trim();
		PRINTLN(content.length());
		uint16_t bufsz = content.length() * 1.01;
		char *buf = new char [bufsz];//[1926];
		char *obuf = new char [bufsz];//[1926]
		content.toCharArray(buf, bufsz);
		int ret = LZ_Compress( (unsigned char*)buf, (unsigned char*)obuf,bufsz);
		PRINTLN(ret);
		String socc = "";
		  for (int i = 0; i < ret; i++ ){
			  //char * o = (char*)obuf;
			  //Serial.print(String(o[i]));
			  String so = String(obuf[i]);
			  //Serial.print(myout[i]);
			  socc = socc + so;

		  }
		//file.println(content);  //save json data
		  PRINT(socc);
		file.println(obuf);
		file.close();
		delete buf;
		delete obuf;
		retVal = true;
	}

	return retVal;
}

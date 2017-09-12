/*
 * Storage.cpp
 *
 *  Created on: 19.05.2017
 *      Author: maro
 */

#include "Storage.h"
#include "Debug.h"

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
	//fs::File f = SPIFFS.open("/wifiset.json", "r");
	//name = "/" + name;
	fs::File f = SPIFFS.open(name, "r");
	if (!f) {
		PRINTLN("wifi config not set/found");
		retVal = "";
	}
	else {  //file exists;
		//retVal = f.readStringUntil('\n');  //read json
		retVal = f.readString();  //read json
		f.close();
	}
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
}

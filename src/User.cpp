/*
 * User.cpp
 *
 *  Created on: 13.08.2017
 *      Author: maro
 */

#include "User.h"
#include <ArduinoJson.h>
#include "Debug.h"

User::User(ESP8266WebServer *server, Storage * storage) {
	_storage = storage;
	_server = server;
	_name = DEFAULT_CONFIG_UN;
	_publicUser = DEFAULT_PUBLIC;
	_password = DEFAULT_CONFIG_PW;

}

User::~User() {
	// TODO Auto-generated destructor stub
}

void User::begin(){

	loadUserConfig();
}

void User::loadUserConfig()
{
	if (loadUserConfigFromStorage() == false) loadDefaultUserConfig();
}

bool User::loadUserConfigFromStorage()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;
	values = _storage->open("/wifiset.json");
	JsonObject& root = jsonBuffer.parseObject(values);  //parse weburl
	if (root.success())
	{
		PRINTLN("in loadUserConfigFromStorage(): wifiset.json found ");
		if (root.containsKey("Public") && root.containsKey("AccessPass"))
		{
			PRINTLN("in loadUserConfigFromStorage(): wifiset.json is valid ");
			if (String(root["Public"].asString()) == "true") _publicUser = true; else  _publicUser = false;
			PRINT("Public user flag: ");PRINTLN(String(root["Public"].asString()));
			_password = root["AccessPass"].asString();
			PRINTLN("in loadUserConfigFromStorage(): all good");
			retVal = true;
		}else{
			PRINTLN("in loadConfigFromStorage(): wifiset.json not valid ");
		}

	}
	else{
		PRINTLN("in loadUserConfigFromStorage(): wifiset.json NOT found ");
	}
	return retVal;
}

void User::loadDefaultUserConfig(){
	_name = DEFAULT_CONFIG_UN;
	_publicUser = DEFAULT_PUBLIC;
	_password = DEFAULT_CONFIG_PW;
}

bool User::isAdmin()
{
	PRINT("isAdmin() password: ");PRINTLN(_password);
	if (_password == "") return true; //not using web password (default);

	bool isAuth = false;
	if (!_server->authenticate(_name.c_str(), _password.c_str()))
	{
		_server->requestAuthentication();

	}
	else isAuth = true;

	return isAuth;
}

bool User::allowPublic(){
	return this->_publicUser;
}

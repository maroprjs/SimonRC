/*
 * User.h
 *
 *  Created on: 13.08.2017
 *      Author: maro
 */

#ifndef USER_H_
#define USER_H_

#include <ESP8266WebServer.h>
#include "Storage.h"

class User {

	bool const DEFAULT_PUBLIC = true;
	String const DEFAULT_CONFIG_UN = "admin";
	String const DEFAULT_CONFIG_PW = "";

public:
	User(ESP8266WebServer *, Storage *);
	virtual ~User();
	void begin();
	bool isAdmin();
	bool allowPublic();

private:
	ESP8266WebServer *_server;
	Storage * _storage;
	bool _publicUser;
	String _name;
	String _password;
private:
	void loadUserConfig();
	bool loadUserConfigFromStorage();
	void loadDefaultUserConfig();
};

#endif /* USER_H_ */

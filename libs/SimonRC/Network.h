/*
 * Network.h
 *
 *  Created on: 10.05.2017
 *      Author: maro
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "Storage.h"
#include "Debug.h"

class Network {

	String const REQUESTED_JSON_VERSION = "v01";
	//String const DEFAULT_DEVICE_NAME = PROJECT_NAME + WiFi.macAddress();
	String const DEFAULT_DEVICE_NAME = PROJECT_NAME;
	String const DEFAULT_SSID = PROJECT_NAME;
	String const DEFAULT_WIFI_PW = PROJECT_NAME;
	bool const DEFAULT_DHCP = true;
	byte const DEF_IP_0 = 192; byte const DEF_IP_1 = 168;
	byte const DEF_IP_2 = 1; byte const DEF_IP_3 = 100;
	byte const DEF_MASK_0 = 255; byte const DEF_MASK_1 = 255;
	byte const DEF_MASK_2 = 255; byte const DEF_MASK_3 = 0;
	byte const DEF_GW_0 = 192; byte const DEF_GW_1 = 168;
	byte const DEF_GW_2 = 1; byte const DEF_GW_3 = 1;
	bool const DEFAULT_CDNS = true;
	bool const DEFAULT_MDNS = false;
	//bool const DEFAULT_ALWAYS_AP = false;
	WiFiMode_t const DEFAULT_WIFI_MODE = WiFiMode_t::WIFI_STA;//initial required mode //:WIFI_STA;(1)//:WIFI_OFF;(0)//:WIFI_AP_STA(3);//:WIFI_AP(2);
	String const DEFAULT_DOMAIN = ".mr";//for dns's
	//->no IP set, will get 192.168.4.1 anyway
	//byte const DEF_AP_IP_0 = 192; byte const DEF_AP_IP_1 = 168;
	//byte const DEF_AP_IP_2 = 2; byte const DEF_AP_IP_3 = 1;
	uint const MAX_WIFI_CONNECT_TIME = 20000;//in [ms] // ... Give ESP 20 seconds to connect to station.

	struct NetworkConnection{
		String deviceName;
		String ssid;
		String wifiPassword;
		bool dhcpInUse;
		byte  ip[4];
		byte  netmask[4];
		byte  gateway[4];
		bool mDNSinUse;
		bool cDNSinUse;
		WiFiMode_t wifiMode;
		//bool permanentAP;
	};

public:


public:
	Network(Storage *, DNSServer*);
	virtual ~Network();
	void begin();
	void handle();
	bool connectionFailed();
	void startDNS();
	void stopDNS();

private:
	Storage * _storage;
	NetworkConnection _nwConnection;
	DNSServer * _dnsServer;
	bool _dnsServerActive;


private:
	void loadNwConfig();
	bool loadNwConfigFromStorage();
	void loadDefaultNwConfig();
	bool isConnected();
	bool initWifiStation();
	void initWifiAP();
};

#endif /* NETWORK_H_ */

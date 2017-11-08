/*
 * Network.cpp
 *
 *  Created on: 10.05.2017
 *      Author: maro
 */

#include "Network.h"

#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

Network::Network(Storage * storage, DNSServer* dnsServer) {
	_storage = storage;
	_dnsServerActive = false;
	_dnsServer = dnsServer;


}

Network::~Network() {
	// TODO Auto-generated destructor stub
}

void Network::begin(){

	loadNwConfig();

	//WIFI INIT
	WiFi.hostname(_nwConnection.deviceName);

	if (_nwConnection.wifiMode == WiFiMode::WIFI_STA){
		WiFi.mode(WIFI_OFF);
		delay(100);
		WiFi.mode(WIFI_STA);
		PRINTLN("Required  WiFi.mode(WIFI_STA)!");
		if (initWifiStation() == false){
			PRINTLN("Connection failed, switching to AP mode");
			WiFi.mode(WIFI_OFF);
			delay(100);
			WiFi.mode(WIFI_AP);
			initWifiAP();
		}

	}
	else if(_nwConnection.wifiMode == WiFiMode::WIFI_AP_STA){
		PRINT("Required  WiFi.mode(WIFI_AP_STA)!");
		WiFi.mode(WIFI_OFF);
		delay(100);
		WiFi.mode(WIFI_AP_STA);
		if (initWifiStation() == false){
			PRINTLN("Connection failed, switching to AP mode");
			WiFi.mode(WIFI_OFF);
			delay(100);
			WiFi.mode(WIFI_AP);
		}
		initWifiAP();
	}
	else{//WiFiMode::WIFI_AP -> WIFI_OFF not considered!!!
		PRINTLN("Required  WiFi.mode(WIFI_AP)!");
		WiFi.mode(WIFI_AP);  //access point only....if no client connect
		initWifiAP();
	}

	//_server.on("/info.html", sendNetworkStatus);TODO
	//server.on("/availnets", SendAvailNetworks);


}

bool Network::initWifiStation(){
	bool retVal = true;
	delay(100);
	PRINT("wifi.hostname: ");PRINTLN(WiFi.hostname());
	WiFi.begin(_nwConnection.ssid.c_str(), _nwConnection.wifiPassword.c_str());
	if(isConnected() == true){
		if (_nwConnection.mDNSinUse == true) {
			String name = _nwConnection.deviceName + DEFAULT_DOMAIN;
			MDNS.begin(name.c_str());  //multicast webname when not in SoftAP mode
			PRINTLN("Starting mDSN " + name);
			MDNS.addService("http", "tcp", 80);
		}
	}
	else{
		PRINTLN("STA Connection failed");
		retVal = false;
	}
	return retVal;
}



void Network::initWifiAP(){
	delay(100);
	//->no IP set, will get 192.168.4.1 anyway
	//WiFi.softAPConfig(IPAddress(DEF_AP_IP_0,DEF_AP_IP_1,DEF_AP_IP_2,DEF_AP_IP_3), IPAddress(DEF_AP_IP_0,DEF_AP_IP_1,DEF_AP_IP_2,DEF_AP_IP_3), IPAddress(255, 255, 255, 0));
	WiFi.softAP(_nwConnection.deviceName.c_str());
	if (_nwConnection.cDNSinUse == true) {   //if captive dns allowed
		startDNS();
	}
	PRINT("Soft AP started IP address: ");
	PRINTLN(WiFi.softAPIP());
}

void Network::startDNS(){
	String dnsName = _nwConnection.deviceName + DEFAULT_DOMAIN;
	_dnsServer->setTTL(300);
	_dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	//_dnsServer.start(53, "*", IPAddress(DEF_AP_IP_0,DEF_AP_IP_1,DEF_AP_IP_2,DEF_AP_IP_3));   //start dns catchall on port 53
	//_dnsServer->start(53, dnsName, WiFi.softAPIP());
	_dnsServer->start(53, "*", WiFi.softAPIP());   //start dns catchall on port 53
	_dnsServerActive = true;
}

void Network::stopDNS(){
	_dnsServer->stop();
	_dnsServerActive = false;
}

bool Network::connectionFailed(){
	PRINT("required wifimode: ");PRINTLN(_nwConnection.wifiMode);
	PRINT("actual wifimode: ");PRINTLN(WiFi.getMode());
	return (((_nwConnection.wifiMode == WiFiMode::WIFI_STA)|| //required some STA, but only AP available
			 (_nwConnection.wifiMode == WiFiMode::WIFI_AP_STA)
			) && (WiFi.getMode() == WiFiMode::WIFI_AP));
}


void Network::handle(){
	if (_dnsServerActive == true)  _dnsServer->processNextRequest();  //captive dns
	yield();
}

void Network::loadNwConfig()
{
	if (loadNwConfigFromStorage() == false) loadDefaultNwConfig();
}

bool Network::loadNwConfigFromStorage()
{

	bool retVal = false;
	String values = "";
	DynamicJsonBuffer jsonBuffer;
	values = _storage->open("/wifiset.json");
	PRINTLN(values);
	JsonObject& root = jsonBuffer.parseObject(values);  //parse weburl
	if (root.success())
	{
		PRINTLN("in loadConfigFromStorage(): wifiset.json found ");
		if (root.containsKey("jsonversion") &&
			root.containsKey("devicename") &&
			root.containsKey("ssid") && root.containsKey("password") &&
			root.containsKey("dhcp") &&
			root.containsKey("ip_0") && root.containsKey("ip_1") &&root.containsKey("ip_2") && root.containsKey("ip_3") &&
			root.containsKey("nm_0") && root.containsKey("nm_1") && root.containsKey("nm_2") && root.containsKey("nm_3") &&
			root.containsKey("gw_0") && root.containsKey("gw_1") && root.containsKey("gw_2") && root.containsKey("gw_3") &&
			//root.containsKey("mDNSoff") && root.containsKey("CDNSoff") &&
			root.containsKey("SoftAP"))
		{
			PRINTLN("in loadConfigFromStorage(): wifiset.json is valid ");
			_nwConnection.deviceName = root["devicename"].asString();
			if (String(root["jsonversion"].asString()) == REQUESTED_JSON_VERSION) { //verify good json info
				_nwConnection.ssid = root["ssid"].asString();
				_nwConnection.wifiPassword = root["password"].asString();
				if (String(root["dhcp"].asString()).toInt() == 1) _nwConnection.dhcpInUse = true; else _nwConnection.dhcpInUse = false;
				_nwConnection.ip[0] = String(root["ip_0"].asString()).toInt(); _nwConnection.ip[1] = String(root["ip_1"].asString()).toInt(); _nwConnection.ip[2] = String(root["ip_2"].asString()).toInt(); _nwConnection.ip[3] = String(root["ip_3"].asString()).toInt();
				_nwConnection.netmask[0] = String(root["nm_0"].asString()).toInt(); _nwConnection.netmask[1] = String(root["nm_1"].asString()).toInt(); _nwConnection.netmask[2] = String(root["nm_2"].asString()).toInt(); _nwConnection.netmask[3] = String(root["nm_3"].asString()).toInt();
				_nwConnection.gateway[0] = String(root["gw_0"].asString()).toInt(); _nwConnection.gateway[1] = String(root["gw_1"].asString()).toInt(); _nwConnection.gateway[2] = String(root["gw_2"].asString()).toInt(); _nwConnection.gateway[3] = String(root["gw_3"].asString()).toInt();
				if (String(root["mDNSoff"].asString()) == "true") _nwConnection.mDNSinUse = false; else  _nwConnection.mDNSinUse = true;
				if (String(root["CDNSoff"].asString()) == "true") _nwConnection.cDNSinUse = false; else  _nwConnection.cDNSinUse = true;
				//if (String(root["SoftAP"].asString()) == "true") _nwConnection.permanentAP = true; else  _nwConnection.permanentAP = false;
				//:WIFI_STA;(1)//:WIFI_OFF;(0)//:WIFI_AP_STA(3);//:WIFI_AP(2);
				_nwConnection.wifiMode = (WiFiMode_t)String(root["SoftAP"].asString()).toInt();

				PRINTLN("in loadConfigFromStorage(): all good");
				retVal = true;
			}else{
				PRINTLN("in loadConfigFromStorage(): json-format incompatible ");
			}
		}else{
			PRINTLN("in loadConfigFromStorage(): wifiset.json not valid ");
		}

	}
	else{
		PRINTLN("in loadConfigFromStorage(): wifiset.json NOT found ");
	}
	return retVal;
}

void Network::loadDefaultNwConfig(){
	_nwConnection.deviceName = DEFAULT_DEVICE_NAME;
	_nwConnection.ssid = DEFAULT_SSID;
	_nwConnection.wifiPassword = DEFAULT_WIFI_PW;
	_nwConnection.dhcpInUse = DEFAULT_DHCP;
	_nwConnection.ip[0] = DEF_IP_0;
	_nwConnection.ip[1] = DEF_IP_1;
	_nwConnection.ip[2] = DEF_IP_2;
	_nwConnection.ip[3] = DEF_IP_3;
	_nwConnection.netmask[0] = DEF_MASK_0;
	_nwConnection.netmask[1] = DEF_MASK_1;
	_nwConnection.netmask[2] = DEF_MASK_2;
	_nwConnection.netmask[3] = DEF_MASK_3;
	_nwConnection.gateway[0] = DEF_GW_0;
	_nwConnection.gateway[1] = DEF_GW_1;
	_nwConnection.gateway[2] = DEF_GW_2;
	_nwConnection.gateway[3] = DEF_GW_3;
	_nwConnection.cDNSinUse = DEFAULT_CDNS;
	_nwConnection.mDNSinUse = DEFAULT_MDNS;
	//_nwConnection.permanentAP = DEFAULT_ALWAYS_AP;
	_nwConnection.wifiMode = DEFAULT_WIFI_MODE;//:WIFI_STA;(1)//:WIFI_OFF;(0)//:WIFI_AP_STA(3);//:WIFI_AP(2);

}

bool Network::isConnected(){
	bool retVal = false;
	unsigned long startTime = millis();
	while (WiFi.status() != WL_CONNECTED && millis() - startTime < MAX_WIFI_CONNECT_TIME &&  !(_nwConnection.ssid=="DEFAULT_SSID"))// ... Give ESP x seconds to connect to station.
	{
		PRINT('.');
		// SerialLog.print(WiFi.status());
		delay(500);
		yield();
	}
	//if (WiFi.waitForConnectResult() != WL_CONNECTED)<-doesn't work well!!!
	//{
	//	PRINTLN("first check, cannot connect to: " + _nwConnection.ssid);
	//	retVal = false;
	//	//return false
	//}else{
	//	retVal = true;
	//}



	//check again if first check was unseccessful:
	if ((retVal == false) && (WiFi.status() == WL_CONNECTED))
	{
		// ... print IP Address
		PRINT("wlan connected, IP address: ");
		PRINTLN(WiFi.localIP());
		retVal = true;
	}

	if ((retVal == true) && (_nwConnection.dhcpInUse == false))	{
		WiFi.config(IPAddress(_nwConnection.ip[0], _nwConnection.ip[1], _nwConnection.ip[2],_nwConnection.ip[3]),
				 	IPAddress(_nwConnection.gateway[0], _nwConnection.gateway[1], _nwConnection.gateway[2], _nwConnection.gateway[3]),
					IPAddress(_nwConnection.netmask[0],_nwConnection.netmask[1], _nwConnection.netmask[2], _nwConnection.netmask[3]));
	}
	return retVal;
}



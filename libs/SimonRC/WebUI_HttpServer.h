/*
 * WebUI_HttpServer.h
 *
 *  Created on: 09.05.2017
 *      Author: maro
 */

#ifndef WEBUI_HTTPSERVER_H_
#define WEBUI_HTTPSERVER_H_
//#include <vector>//MRO: use this before #include <ESP8266WebServer.h>, otherwise comple error
//(->https://github.com/esp8266/Arduino/issues/2549)
//#include <ESP8266WebServer.h>
//#include <DNSServer.h>
//#include "Storage.h"
//#include "Network.h"
//#include "User.h"
#include "HttpServer.h"

class WebUI_HttpServer {
public:

	//typedef std::function<void()> customFunction_t;	//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
	//typedef std::vector <customFunction_t> customFunctionList_t;
	//typedef std::vector <String> customServerPathList_t;
    //struct CustomFunctionRecord
    //{
	//	customFunction_t  customFunction;
	//    String customServerPath;
    //} ;
    //typedef CustomFunctionRecord customFunctionRecord_t;
	//typedef std::vector <customFunctionRecord_t> customFunctionRecordVector_t;


	//WebUI_HttpServer();
	WebUI_HttpServer(HttpServer *);
	virtual ~WebUI_HttpServer();
	void begin();
	void handle();


	//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
	//void addCustomFunction(customFunction_t cb, String serverPath);
	//void handleCustomFunctions();
	//customFunctionList_t _customFunctionList;
    //typedef void result_type;//TODO: try to understand this (compiler error if this is not there)
    //void operator() (customFunctionRecord_t cb) { cb(); };
    //customServerPathList_t _customServerPathList;
    //customFunctionRecordVector_t _customFunctionRecordVector;
    //customFunctionRecord_t _customFunctionRecord;
protected:
	HttpServer *_server;
	//Storage * _storage;
	//User * _user;
	//Network * _network;
	File _fsUploadFile;
	//DNSServer * _dnsServer;
	//void (*customFunction_)() = NULL;
	//callback_vector m_cb;


private:
	bool isPublicFile(String filename);
	unsigned char h2int(char c);
	String urldecode(String input) ;
	bool handleFileDownload(String fname);
	String getContentType(String filename);
	void handleFileDelete(String fname);
	void handleFileBrowser();
	void handleRoot();
	void handleRoot2();
	void formatspiffs();
	void handleWifiConfig();
	bool handleFileRead(String path);
	void handleFileUpload();
	void handleFileList();
	void handleJsonSave();
	void handleJsonLoad();
	void CheckNewSystem();
	void FileSaveContent_P(String fname, PGM_P content, u_long numbytes, bool overWrite);

};

#endif /* WEBUI_HTTPSERVER_H_ */

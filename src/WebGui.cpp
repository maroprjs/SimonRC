/*
 * WebGui.cpp
 *
 *  Created on: 09.05.2017
 *      Author: maro
 */

#include "WebGui.h"
#include "Debug.h"
#include<algorithm> //needed for std:for_each(..); ref: http://stackoverflow.com/questions/30706652/for-each-is-not-a-member-of-std-c11-support-already-enabled

#include "htmlEmbed.h"



WebGui::WebGui(ESP8266WebServer *server, Storage *storage, User* user,Network * network, DNSServer * dnsServer) {
	_server = server;
	_storage = storage;
	_user = user;
	_network = network;
	_dnsServer = dnsServer;

}
//WebGui::WebGui() {

//}

WebGui::~WebGui() {
	// TODO Auto-generated destructor stub
}

void WebGui::begin() {
	SHOWSPIFFS;//TODO: include spiffs.begin also in non debug modus
	//from: https://community.platformio.org/t/esp8266webserver-inside-custom-class/237
	//_server->onFileUpload(std::bind(&WebGui::handleRoot2, this));
	_server->on("/",std::bind(&WebGui::handleRoot, this));
	_server->on("/favicon.ico", [this]() { _server->send(200, "text/html", "");   });
	_server->on("/browse", std::bind(&WebGui::handleFileBrowser, this));
	_server->on("/upload", HTTP_POST, [this]() { _server->send(200, "text/plain", ""); }, std::bind(&WebGui::handleFileUpload, this));
	_server->on("/jsonsave", std::bind(&WebGui::handleJsonSave, this));
	_server->on("/jsonload", std::bind(&WebGui::handleJsonLoad, this));
	_server->on("/formatspiff", std::bind(&WebGui::formatspiffs, this));
	//_server->on("/generate_204",std::bind(&WebGui::handleRoot2, this));
	  //server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
	  //server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
	_server->onNotFound([this](){
		PRINTLN("_server->onNotFound([this]()");
		_network->stopDNS();
		PRINT("server uri: ");PRINTLN(_server->uri());
		if(!handleFileRead(_server->uri()))_server->send(404, "text/plain", "FileNotFound");
		_network->startDNS();
	});

	CheckNewSystem();
	PRINTLN("HTTP server started");


}

void WebGui::formatspiffs()
{
	if (_user->isAdmin() == false) return;
	PRINTLN("formatting spiff...");
	if (!SPIFFS.format()) {
		PRINTLN("Format failed");
	}
	else { PRINTLN("format done...."); }
	_server->send(200, "text/html", "Format Finished....rebooting");
}

void WebGui::handleRoot2(){
	//PRINTLN("204 generated");
	_network->stopDNS();
	_dnsServer->stop();

}
void WebGui::handleRoot(){//handles root of website (used in case of virgin systems.)

	_network->stopDNS();
	  if (!handleFileRead("/")) {   //if new system without index we either show wifisetup or if already setup/connected we show filebrowser for config.
		  PRINTLN("handleRoot() no index.html found");
		  if (_user->isAdmin())
		  {
	    	 if (_network->connectionFailed()) {
	    		 PRINTLN("calling  handleWifiConfig();");
	    		 handleWifiConfig();
	    	 } else {
	    		 PRINTLN("calling  handleFileBrowser()");
	    		 handleFileBrowser();
	    	 }
	      }
	  }
	  _network->startDNS();

}

void WebGui::handleWifiConfig() {
	//send GZ version of embedded config
	_server->sendHeader("Content-Encoding", "gzip");
	_server->send_P(200, "text/html", PAGE_WIFISETUP, sizeof(PAGE_WIFISETUP));
}

void WebGui::handleFileBrowser(){
	  if (_user->isAdmin() == false) return;
	  if (_server->arg("do") == "list") {
	    handleFileList();
	  }
	  else
	    if (_server->arg("do") == "delete") {
	      handleFileDelete(_server->arg("file"));
	    }
	    else
	      if (_server->arg("do") == "download") {
	        handleFileDownload(_server->arg("file"));
	      }
	      else
	      {
	        if (!handleFileRead("/filebrowse.html")) { //send GZ version of embedded browser
	        						PRINTLN("calling handleFileRead(/filebrowse.html) unsuccessful");
	                                _server->sendHeader("Content-Encoding", "gzip");
	                                _server->send_P(200, "text/html", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE));
	                             }
	        //TODO: implment semaphore ->MyWebServer.isDownloading = true; //need to stop all cloud services from doing anything!  crashes on upload with mqtt...
	      }

}

void WebGui::handleFileList()
{
	PRINTLN("handleFileList() called");
	if (_user->isAdmin() == false) return;
	Dir dir = SPIFFS.openDir("/");//TODO: use storage

	String output = "{\"success\":true, \"is_writable\" : true, \"results\" :[";
	bool firstrec = true;
	while (dir.next()) {
		if (!firstrec) {
			output += ','; //add comma between recs (not first rec)
		}
		else {
			firstrec = false;
		}
		String fn = dir.fileName();
		fn.remove(0, 1); //remove slash
		output += "{\"is_dir\":false";
		output += ",\"name\":\"" + fn;
		output += "\",\"size\":" + String(dir.fileSize());
		output += ",\"path\":\"";
		output += "\",\"is_deleteable\":true";
		output += ",\"is_readable\":true";
		output += ",\"is_writable\":true";
		output += ",\"is_executable\":true";
		output += ",\"mtime\":1452813740";   //dummy time
		output += "}";
	}
	output += "]}";
	//PRINTLN("got list >"+output);
	_server->send(200, "text/json", output);
}

void WebGui::handleFileDelete(String fname)
{
	PRINTLN("handleFileDelete() called");
	if (_user->isAdmin() == false) return;
	PRINTLN("handleFileDelete: " + fname);
	fname = '/' + fname;
	fname = urldecode(fname);
	if (!SPIFFS.exists(fname)) return _server->send(404, "text/plain", "FileNotFound");
	if (SPIFFS.exists(fname))//TODO: use storage
	{
		SPIFFS.remove(fname);
		_server->send(200, "text/plain", "");
	}
}

bool WebGui::handleFileDownload(String fname)
{
  PRINTLN("handleFileDownload: " + fname);
  String contentType = "application/octet-stream";
  fname = "/" + fname;
  fname = urldecode(fname);
  if (isPublicFile(fname) == false)
  {
    if (_user->isAdmin() == false) return false;
  }  //check if a public file.

  if (SPIFFS.exists(fname)) {//TODO: use storage
    File file = SPIFFS.open(fname, "r");
    _server->streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

bool WebGui::handleFileRead(String path){
	PRINTLN("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	path = urldecode(path);
	if (isPublicFile(path) == false) {
		if (_user->isAdmin() == false) return false;  //check if a public file.
	}
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {//TODO: storage
			if (SPIFFS.exists(pathWithGz)) path += ".gz";
			File file = SPIFFS.open(path, "r");
			PRINT("filesize: ");PRINTLN(file.size());
			_server->streamFile(file, contentType);
			file.close();
			return true;
	}
	return false;
}

void WebGui::handleFileUpload(){
	  if (_server->uri() != "/upload") return;
	  if (_user->isAdmin() == false) return;
	  //TODO: implement semaphore MyWebServer.isDownloading = true;
	  HTTPUpload& upload = _server->upload();
	  if (upload.status == UPLOAD_FILE_START) {
	    String filename = upload.filename;
	    if (!filename.startsWith("/")) filename = "/" + filename;
	    PRINTLN("handleFileUpload Name: "); PRINTLN(filename);
	    _fsUploadFile = SPIFFS.open(filename, "w");
	    filename = String();
	  }
	  else if (upload.status == UPLOAD_FILE_WRITE) {
	    //SerialLog.print("handleFileUpload Data: "); DebugPrintln(upload.currentSize);
	    if (_fsUploadFile) _fsUploadFile.write(upload.buf, upload.currentSize);
	  }
	  else if (upload.status == UPLOAD_FILE_END) {
	    if (_fsUploadFile) _fsUploadFile.close();
	    PRINT("handleFileUpload Size: "); PRINTLN(upload.totalSize);
	//    MyWebServer.isDownloading = false;
	  }

}


void WebGui::handleJsonSave()
{

	if (_server->args() == 0)
		return _server->send(500, "text/plain", "BAD JsonSave ARGS");

	String fname = "/" + _server->arg(0);
	fname = urldecode(fname);

	PRINTLN("handleJsonSave: " + fname);

	if (isPublicFile(fname) == false)
	{
		if (_user->isAdmin() == false) return;  //check if a public file.
	}

	File file = SPIFFS.open(fname, "w");//TODO_ move to storage
	if (file) {
		file.println(_server->arg(1));  //save json data
		file.close();
	}
	else  //cant create file
		return _server->send(500, "text/plain", "JSONSave FAILED");
	_server->send(200, "text/plain", "");

	//if (jsonSaveHandle != NULL)	jsonSaveHandle(fname);TODO: implement this
}

void WebGui::handleJsonLoad()
{

	if (_server->args() == 0)
		return _server->send(500, "text/plain", "BAD JsonLoad ARGS");
	String fname = "/" + _server->arg(0);

	fname = urldecode(fname);
	PRINTLN("handleJsonRead: " + fname);

	if (isPublicFile(fname) == false)
	{
		if (_user->isAdmin() == false) return;  //check if a public file.
	}

	File file = SPIFFS.open(fname, "r");//TODO:storage
	if (file) {
		_server->streamFile(file, "application/json");
		file.close();
	}
}

void WebGui::CheckNewSystem() {   //if new system we save the embedded htmls into the root of Spiffs as .gz!

	FileSaveContent_P("/wifisetup.html.gz", PAGE_WIFISETUP, sizeof(PAGE_WIFISETUP), false);
	FileSaveContent_P("/filebrowse.html.gz", PAGE_FSBROWSE, sizeof(PAGE_FSBROWSE), false);
	FileSaveContent_P("/jquery-1.8.2.min.js.gz", SRC_JQUERY, sizeof(SRC_JQUERY), false);

}

void WebGui::FileSaveContent_P(String fname, PGM_P content, u_long numbytes, bool overWrite = false) {   //save PROGMEM array to spiffs file....//f must be already open for write!

	if (SPIFFS.exists(fname) && overWrite == false) return;//TODO: storage


	const int writepagesize = 1024;
	char contentUnit[writepagesize + 1];
	contentUnit[writepagesize] = '\0';
	u_long remaining_size = numbytes;


	File f = SPIFFS.open(fname, "w");



	if (f) { // we could open the file

		while (content != NULL && remaining_size > 0) {
			size_t contentUnitLen = writepagesize;

			if (remaining_size < writepagesize) contentUnitLen = remaining_size;
			// due to the memcpy signature, lots of casts are needed
			memcpy_P((void*)contentUnit, (PGM_VOID_P)content, contentUnitLen);

			content += contentUnitLen;
			remaining_size -= contentUnitLen;

			// write is so overloaded, had to use the cast to get it pick the right one
			f.write((uint8_t *)contentUnit, contentUnitLen);
		}
		f.close();
		PRINTLN("created:" + fname);
	}
}

//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
//void WebGui::addCustomFunction(customFunction_t cb, String serverPath){
	 //_customFunctionList.push_back(cb);
	// _customServerPathList.push_back(serverPath);
	//_customFunctionRecord.customFunctionList.push_back(cb);
	//_customFunctionRecord.customServerPathList.p
	//_customFunctionRecord.customFunction = cb;
	//_customFunctionRecord.customServerPath = serverPath;
	//_customFunctionRecordVector.push_back(_customFunctionRecord);
	//_server->on((const char*)cfr->customServerPath.c_str(), cfr->customFunction);
	//_server->on((const char*)serverPath.c_str(), cb);

//}

//reference: http://stackoverflow.com/questions/3257896/c-for-each-calling-a-vector-of-callback-functions-and-passing-each-one-an-argu
//void WebGui::handleCustomFunctions() {
	//int i = 0;
//	PRINTLN("in handleCustomFunctions");
//	std::vector<customFunctionRecord_t>::const_iterator cfr;
	//std::for_each(_customFunctionList.begin(), _customFunctionList.end(), std::ref(*this));
//    for(cfr=_customFunctionRecordVector.begin(); cfr!=_customFunctionRecordVector.end(); ++cfr){
    	//_server->on(cfr.customServerPath, std::bind(&WebGui::handleCustomFunctions, this));
 //   	PRINTLN(cfr->customServerPath);
 //   	_server->on((const char*)cfr->customServerPath.c_str(), cfr->customFunction);
 //   }
//}


bool WebGui::isPublicFile(String filename)  //in Public mode,  display any file that doesn't have a $$$, or they will need admin access....
{
	bool isPub = false;

	if (_user->allowPublic())
	{
		if (filename.indexOf("$$$") < 0) isPub = true;   //if no $ in filename then allow file to be used/viewed.
		if (filename.indexOf("wifiset") >= 0) isPub = false;   //hardcode wifiset cannot be public.....
	}

	return isPub;
}

String WebGui::getContentType(String filename){
  if(_server->hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String WebGui::urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
  char c;
  String ret = "";

  for (byte t = 0; t<input.length(); t++)
  {
    c = input[t];
    if (c == '+') c = ' ';
    if (c == '%') {


      t++;
      c = input[t];
      t++;
      c = (h2int(c) << 4) | h2int(input[t]);
    }

    ret.concat(c);
  }
  yield();//TODO: check if sesnible from https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
  return ret;

}


// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char WebGui::h2int(char c)
{
  if (c >= '0' && c <= '9') {
    return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return((unsigned char)c - 'A' + 10);
  }
  return(0);
}










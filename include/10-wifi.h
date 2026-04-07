#ifndef __PROTO_10_WIFI_CPP__
#define __PROTO_10_WIFI_CPP__
//Extracted Prototyes
// ****************************
// src/10-wifi.cpp prototypes
// ****************************
void setup();
void loop();
void handleRoot();
void handleSave();
String readStringFromEEPROM(int addr);
void writeStringToEEPROM(int addr, String data);
#endif

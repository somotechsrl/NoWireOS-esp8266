#ifndef __PROTO_20_MQTT_CUBE_CPP__
#define __PROTO_20_MQTT_CUBE_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt-cube.cpp prototypes
// ****************************
void mkDevKeys();
void mqttInit();
void mqttUp();
void mqttRpcUp(String rpcid);
bool netCheck();
void checkReboot();
bool mqttPoll();
#endif

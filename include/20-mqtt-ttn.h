#ifndef __PROTO_20_MQTT_TTN_CPP__
#define __PROTO_20_MQTT_TTN_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt-ttn.cpp prototypes
// ****************************
void downLinkDataHandle(McpsIndication_t *mcpsIndication);
void netInit();
void mqttInit();
bool netCheck();
bool mqttPoll();
void mqttUp();
void mqttRpcUp(String responseID);
#endif

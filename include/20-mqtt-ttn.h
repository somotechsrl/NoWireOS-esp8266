#ifndef __PROTO_20_MQTT_TTN_CPP__
#define __PROTO_20_MQTT_TTN_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt-ttn.cpp prototypes
// ****************************
void mkDevKeys();
void downLinkDataHandle(McpsIndication_t *mcpsIndication);
void mqttUp(uint8_t port);
void mqttRpcUp(String responseID);
void mqttInit();
void mqttLoop();
#endif

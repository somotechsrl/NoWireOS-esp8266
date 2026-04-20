#ifndef __PROTO_20_MQTT_WIFI_CPP__
#define __PROTO_20_MQTT_WIFI_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt-wifi.cpp prototypes
// ****************************
void mqttInit();
bool mqttPoll();
void mqttUp();
void mqttRpcUp(String responseID);
#endif

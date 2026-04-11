#ifndef __PROTO_20_MQTT_CPP__
#define __PROTO_20_MQTT_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt.cpp prototypes
// ****************************
void messageReceived(String &topic, String &payload);
void mqttInit();
void mqttPoll();
void mqttUp();
void mqttRpcUp(String subtopic,bool sync);
void mqttDebugUp(const char *json);
#endif

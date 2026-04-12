#ifndef __PROTO_20_MQTT_CPP__
#define __PROTO_20_MQTT_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt.cpp prototypes
// ****************************
static void messageReceived(String &topic, String &payload);
static void mqttReconnect();
void mqttInit();
void mqttPoll();
static void mqttSend(const char * topic, const char *data);
void mqttUp();
void mqttRpcUp(String responseID);
#endif

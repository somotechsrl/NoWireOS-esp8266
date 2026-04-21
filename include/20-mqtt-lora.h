#ifndef __PROTO_20_MQTT_LORA_CPP__
#define __PROTO_20_MQTT_LORA_CPP__
//Extracted Prototyes
// ****************************
// src/20-mqtt-lora.cpp prototypes
// ****************************
void onDownlinkReceived(McpsIndication_t *mcpsIndication);
void downLinkDataHandle(McpsIndication_t *m);
void netInit();
void mqttInit();
bool netCheck();
bool mqttPoll();
void mqttUp();
void mqttRpcUp(String responseID);
void wifiReset();
#endif

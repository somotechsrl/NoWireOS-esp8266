#ifndef __PROTO_Z20_MQTT_INO__
#define __PROTO_Z20_MQTT_INO__
//Extracted Prototyes
// ****************************
// src/Z20_MQTT.ino prototypes
// ****************************
void messageReceived(String &topic, String &payload);
void mqttInit();
void mqttPoll();
void mqttUp();
void mqttRpcUp(String subtopic,bool sync);
void mqttDebugUp(const char *json);
void otaUpdate();
#endif

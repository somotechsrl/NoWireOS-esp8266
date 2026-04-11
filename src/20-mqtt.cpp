#include "main.h"
#include "rBase64.h"
#include "MQTT.h"
#include <WiFiClientSecure.h>

#ifdef MQTT_TASK

#define TAG MQTT
static WiFiClientSecure snet;

static const char *broker = "rpc.somotech.it";
static uint16_t port=8893;
#define TSIZE 128
static char topic[TSIZE];

// topics (uuid is defined HERE)
#define DBG_MQTT "MQTT"

// Uses Crypted NET (inseCure with unk certificate, but anyway crypted...)
static MQTTClient mqttClient(BUFSIZE);
void messageReceived(String &topic, String &payload) {

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.

  // must find a way to handle async calls, even if not necessary...
  //debug(DBG_MQTT,String("Received topic: ")+topic);
  //debug(DBG_MQTT,String("Received message: ")+payload);

  if(strstr(topic.c_str(),"rpc")) {
    rpcManage(payload,true); return;
    }
  if(strstr(topic.c_str(),"asy")) {
    rpcManage(payload,false); return;
    }

  //debug(DBG_MQTT,String("Not Handled"));

}


void mqttInit() {

  netInit();

  snet.setInsecure();

  mqttClient.begin(broker,port, snet);
  mqttClient.onMessage(messageReceived);

  // first connect...
  mqttReconnect();
  }

static void mqttReconnect() {

  if (mqttClient.connected()) return;

  //debug(DBG_MQTT, "Connecting...");
  while (!mqttClient.connect(clientId.c_str())) {
    //debug(DBG_MQTT, "Failed: RC="+mqttClient.lastError());
    delay(1000);
  }

  snprintf(topic, TSIZE, "nowireos/%s/%s/rpc", BOARDID, uuid);
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/asy", BOARDID, uuid);
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/down", BOARDID, uuid);
  mqttClient.subscribe(topic);
}

void mqttPoll() {
  mqttReconnect();
  mqttClient.loop();
}

// commond publish function
static void mqttSend(const char * topic, const char *data) {
  mqttReconnect();
  // flashes yellow /(send)
  mqttClient.publish(topic,data,strlen(data));
}

// commond publish function
void mqttUp() {
  // Magic calculation on expanded buffer
  char mbuff[16];
  uint16_t randseed=rand();
  uint16_t magic=randseed ^ 0x1b2c;
  sprintf(topic,"nowireos/%s/%s/up/%04x%04x",uuid,BOARDID,randseed,magic);
  mqttSend(topic);
}

void mqttRpcUp(String subtopic,bool sync) {
    snprintf(topic, TSIZE, "nowireos/%s/%s/%s/%s", BOARDID, uuid, (sync ? "rpc" : "asy"), subtopic.c_str());
    mqttSend(topic);
}

void mqttDebugUp(const char *json) {
  reconnect();
  mqttClient.publish(dbg.c_str(), json, strlen(json));
}



#endif
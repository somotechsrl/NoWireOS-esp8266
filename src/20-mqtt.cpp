#include "main.h"
#include "rBase64.h"
#include "MQTT.h"
#include <WiFiClientSecure.h>
#include "20-mqtt.h"
#include "10-encoder.h"
#include "00-debug.h"
#include "20-rpc-manager.h"

// Default server and port, can be overridden by config or other means
static const char *broker = "rpc.somotech.it";
static uint16_t port=8893;
static WiFiClientSecure snet;


#define TAG "MQTT"
#define TSIZE 128
static char topic[TSIZE];

// uuid and mac
String uuid, mac;
static void netInit() {

  WiFi.begin();
  uuid = mac = WiFi.macAddress();
  ESP_LOGI(TAG, "MAC: %s", mac.c_str());
  uuid.replace(":", "");
  ESP_LOGI(TAG, "UUID: %s", uuid.c_str());

}


// Uses Crypted NET (inseCure with unk certificate, but anyway crypted...)
static MQTTClient mqttClient(BUFSIZE);
static void messageReceived(String &topic, String &payload) {

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.

  // must find a way to handle async calls, even if not necessary...
  //debug(DBG_MQTT,String("Received topic: ")+topic);
  //debug(DBG_MQTT,String("Received message: ")+payload);
  ESP_LOGI(TAG, "Received topic: %s", topic.c_str());
  ESP_LOGI(TAG, "Received message: %s", payload.c_str());

  if(topic.indexOf("rpc") != -1) {
    rpcManage(payload.c_str(),true); 
    return;
    }
  if(topic.indexOf("asy") != -1) {
    rpcManage(payload.c_str(),false); 
    return;
    }

  ESP_LOGI(TAG, "No handler for topic: %s", topic.c_str());
}

static void mqttReconnect() {

  if (mqttClient.connected()) return;

  uint32_t timeout=millis()+2000;

  //debug(DBG_MQTT, "Connecting...");
  while (!mqttClient.connect(broker,port) && millis()<timeout) {
    ESP_LOGE(TAG, "Failed to connect to MQTT broker at %s:%d -- RC=%d", broker, port,mqttClient.lastError());
    //debug(DBG_MQTT, "Failed: RC="+mqttClient.lastError());
    delay(500);
  }

  snprintf(topic, TSIZE, "nowireos/%s/%s/rpc", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/asy", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/down", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);
}

void mqttInit() {

  netInit();

  snet.setInsecure();

  mqttClient.begin(broker,port, snet);
  mqttClient.onMessage(messageReceived);

  // first connect...
  // mqttReconnect();
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
  uint16_t randseed=rand();
  uint16_t magic=randseed ^ 0x1b2c;
  sprintf(topic,"nowireos/%s/%s/up/%04x%04x",BOARDID,uuid.c_str(),randseed,magic);
  mqttSend(topic,jsonGetBase64());
}

void mqttRpcUp(String responseID) {
    ESP_LOGI(TAG, "Publishing RPC response with ID: %s", responseID.c_str());
    snprintf(topic, TSIZE, "nowireos/%s/%s/rpc/%s", BOARDID, uuid.c_str(), responseID.c_str());
    mqttSend(topic,jsonGetBase64());
}

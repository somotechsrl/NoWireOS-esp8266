#include "NowireOS.h"
#include "rBase64.h"
#ifdef __MQTT__
#include <MQTT.h>

#ifdef __ESPWIFI__
#include <WiFiClientSecure.h>
static WiFiClientSecure snet;
static uint16_t port=8893;
#endif


// topics (uuid is defined HERE)
String uuid;
static String up, rpc, dbg,down, asy, pfx = "nowireos/";
#define DBG_MQTT "MQTT"

// MQTT Parameters -- hardcoded 
static const char *broker = "rpc.somotech.it";

// Uses Crypted NET (inseCure with unk certificate, but anyway crypted...)
static MQTTClient mqttClient(BUFSIZE);

void messageReceived(String &topic, String &payload) {

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.

  // must find a way to handle async calls, even if not necessary...
  debug(DBG_MQTT,String("Received topic: ")+topic);
  debug(DBG_MQTT,String("Received message: ")+payload);

  commInPixel();
  if(strstr(topic.c_str(),"rpc")) {rpcManage(payload,true); return;}
  if(strstr(topic.c_str(),"asy")) {rpcManage(payload,false); return;}

  debug(DBG_MQTT,String("Not Handled"));

}


void mqttInit() {

  netInit();

  // sets topics
  String fpfx=pfx + String(BOARDID) + "/" + uuid;
  
  up =   fpfx + "/up";
  rpc =  fpfx + "/rpc";
  asy =  fpfx + "/asy";
  dbg  = fpfx + "/dbg";
  down = fpfx + "/down";

#ifdef __ESPWIFI__
  snet.setInsecure();
#endif
#ifdef __ESPETH01__
  snet.setInsecure();
#endif
  mqttClient.begin(broker,port, snet);
  mqttClient.onMessage(messageReceived);

  // first connect...
  reconnect();

}

static void reconnect() {

  if (mqttClient.connected()) return;

  // signals error
  errorPixel();

  String clientId = String(BOARDID) + "-" + uuid;

  debug(DBG_MQTT, "Connecting...");
  while (!mqttClient.connect(clientId.c_str())) {
    debug(DBG_MQTT, "Failed: RC="+mqttClient.lastError());
    delay(1000);
  }
  debug(DBG_MQTT, "Connected!");

  mqttClient.subscribe(rpc.c_str());
  mqttClient.subscribe(asy.c_str());
  mqttClient.subscribe(down.c_str());
}

void mqttPoll() {
  reconnect();
  mqttClient.loop();
}

// commond publish function
static void mqttSend(String topic) {
  
  reconnect();

  const char *json=jsonGetBuffer();
  uint16_t ssize=jsonGetBufferSize(),csize=jsonGetCompressedSize();

  debug(DBG_MQTT,topic+" -> "+json);

  // nothing to send
  if(!ssize || ! csize) return;

  debug(DBG_MQTT,
    String("Bytes: ")
   +String("json=")+String(ssize)
   +String(" compressed=")+String(csize)
   +String(" ratio=")+String((100.*csize)/ssize)
   );
   
   
  // uses compressed jsonbuffer for send!!!!!
  rbase64.encode((uint8_t *)jsonGetEncryptedBuffer(),csize);
  const char *b=rbase64.result();

  // flashes yellow /(send)
  commOutPixel();
  mqttClient.publish(topic.c_str(),b,strlen(b));

  // Clears buffers to avoid false send....
  jsonClear();

}

// commond publish function
void mqttUp() {
  // Magic calculation on expanded buffer
  char mbuff[16];
  uint16_t randseed=rand();
  uint16_t magic=randseed ^ 0x1b2c;
  sprintf(mbuff,"%04x%04x",randseed,magic);
  mqttSend(up + "/" + mbuff);
}

void mqttRpcUp(String subtopic,bool sync) {
    mqttSend((sync ? rpc : asy ) + "/" + subtopic);
   
}

void mqttDebugUp(const char *json) {
  reconnect();
  mqttClient.publish(dbg.c_str(), json, strlen(json));
}

#endif


// Ota subsystem Call, hera as we use httpclientSecure
#ifdef __ESPOTA__

#include <Update.h>

void otaUpdate() {

  WiFiClientSecure client;
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, "hub.somotech.it", 443, "/esp/update/arduino.php", "optional current version string here");
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("[update] Update failed.");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[update] Update no Update.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[update] Update ok."); // may not be called since we reboot the ESP
      break;
  }
  
}

#endif

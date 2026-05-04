// Cube Cell uses only LoRa, no wif, no mqtt, so this module is only compiled if CUBE_CELL is not defined, allowing for a clean separation of WiFi/MQTT functionality and LoRa functionality in the codebase, and ensuring that the appropriate code is included based on the target platform and its capabilities.
#ifndef LORAWAN_CLIENT

#include "main.h"
#include "rBase64.h"
#include "MQTT.h"
#include "10-encoder.h"
#include "20-rpc-manager.h"
#include "10-wifi-provision.h"

/*

Notes on this module:

  Entry point is mqttInit() called in setup(), which initializes the MQTT client and sets the message callback. The actual connection to the MQTT broker is handled in mqttPoll(), which is called in the main loop after checking for WiFi connectivity. This ensures that the MQTT connection is established only after WiFi is connected, and allows for automatic reconnection if the connection drops.
  send/receive functions are implemented as mqttUp() for sending data to the broker, and messageReceived() as the callback for handling incoming messages. The messageReceived() function checks the topic of incoming messages and sets a flag if it's an RPC request, which is then handled in the main loop to avoid potential deadlocks from trying to publish from within the MQTT callback. This allows for asynchronous handling of RPC requests and responses without blocking the MQTT client.
  The mqttReconnect() function is responsible for establishing a connection to the MQTT broker, and includes retry logic with a timeout to prevent infinite blocking if the broker is unreachable. It also handles subscribing to the necessary topics once a
  connection is established. This function is called from mqttPoll() to ensure that the MQTT client is always connected when trying to publish or handle messages.  

  Working on a Lora version with work almost same way, but using lora instead of wifi, and a different MQTT client library that supports Lora communication, this will allow for more flexible deployment options in environments where WiFi is not available or reliable, while still maintaining the same MQTT-based communication pattern for consistency across different network types.

  #define USEWIFI is used to conditionally compile the MQTT-related code, allowing for easy exclusion of this functionality in builds where WiFi is not needed or available, such as in a Lora-only version of the firmware. This helps to keep the codebase clean and efficient by only including the necessary components for each specific deployment scenario.

*/

// Default server and port, can be overridden by config or other means
//static const char *broker = "rpc.somotech.it";
//static uint16_t port=8893;
static const char *broker = "rpc.somotech.it";
static uint16_t port=1883;

static WiFiClient cnet;
static WiFiClientSecure snet;


#define TAG "MQTT"
#define TSIZE 128
static char topic[TSIZE];

struct RpcResponse {
    bool active=false;
    char topic[TSIZE] ;
    char message[BUFSIZE];
} rpcResp;

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

  // sets response for RPC calls, then handled in loop to publish response, this is needed to avoid deadlocks in MQTT callback when trying to publish from within the callback itself, allows for more flexible handling of responses and avoids potential issues with MQTT client state during callback execution
  if(topic.indexOf("rpc") != -1) {
    rpcResp.active=true;
    strcpy(rpcResp.topic, topic.c_str());
    strcpy(rpcResp.message, payload.c_str());
    return;
    }

  ESP_LOGI(TAG, "No handler for topic: %s", topic.c_str());
}

static bool mqttReconnect() {

   // already connected, no need to reconnect
  if(mqttClient.connected()) return true;

   // no wifi...
  if(!netCheck()) return false;

  uint8_t attempt=0;
  uint32_t timeout=millis()+2000;
  String clientId = String(BOARDID) + "_" + String(uuid);
  ESP_LOGI(TAG, "Connecting to MQTT broker at %s:%d with client ID: %s", broker, port, clientId.c_str());
  while (!mqttClient.connect(clientId.c_str()) && millis()<timeout) {
    ledToggle();
    attempt++;
    delay(200);
    }

  // check connection status 
  if (!mqttClient.connected()) {
    ESP_LOGE(TAG, "Failed to connect to MQTT broker at %s:%d after %d attempts", broker, port, attempt);
    return false;
    }

  // subscribe to topics, can be extended later to include more topics or wildcard subscriptions as needed for more flexible communication patterns
  ESP_LOGI(TAG, "Connected to MQTT broker at %s:%d -- subscribing to topics", broker, port);
  ESP_LOGI(TAG, "Subscribing to topic: nowireos/%s/%s/rpc", BOARDID, uuid.c_str());
  ESP_LOGI(TAG, "Subscribing to topic: nowireos/%s/%s/asy", BOARDID, uuid.c_str());
  ESP_LOGI(TAG, "Subscribing to topic: nowireos/%s/%s/down", BOARDID, uuid.c_str());
  snprintf(topic, TSIZE, "nowireos/%s/%s/rpc", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/asy", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);
  snprintf(topic, TSIZE, "nowireos/%s/%s/down", BOARDID, uuid.c_str());
  mqttClient.subscribe(topic);

  // connected and subscribed successfully
  ESP_LOGI(TAG, "Successfully connected and subscribed to MQTT broker at %s:%d", broker, port);
  return true;
}

// initialize MQTT client and set callback, connection is handled in loop() to ensure it happens after WiFi is connected
void mqttInit() {
  ESP_LOGI(TAG, "Initializing MQTT client...");
  snet.setInsecure();
  mqttClient.begin(broker,port,cnet);
  mqttClient.onMessage(messageReceived);
  }

bool mqttPoll() {
  if(!mqttReconnect()) return false;
  // RPC response handling, checks if there's an active RPC response to send, if so, publishes it to the appropriate topic and resets the response state, this allows for asynchronous handling of RPC responses without blocking the MQTT callback
  if(rpcResp.active) {
    ESP_LOGI(TAG, "Handling RPC resquest %s %s", rpcResp.topic, rpcResp.message);
    rpcManage(rpcResp.message, true);
    rpcResp.active=false;
    }
  mqttClient.loop();
  return true;
}

// commond publish function
static void mqttSend(const char * topic, const char *data) {
  mqttReconnect();
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

#endif

#include "00-main.h"
#ifdef CUBE_CELL_OFF

#include "00-debug.h"
#include <LoRaWan_APP.h>
#include <cppQueue.h>
#include "innerWdt.h"
#include "10-encoder.h"
#include "20-mqtt-cube.h"


#define TAG "LORA"

/**************************************************************************

   Emulates mqtt behaviour to LoRaWan Gateway
   Data are sent and pulled from somotech portal in nwos subsystem
   Magic is achieved using fport 10,11,20,21,22
   (C) Somotech, 2021

**************************************************************************/

// Standard Function port
#define FPORT_CFG 10
#define FPORT_EVT 11
#define FPORT_STD 20
#define FPORT_RPC 21
#define FPORT_RPC_ASYNC 22
#define MIN_DUTYCYCLE 15000
#define PAYLOAD_SIZE 128
#define QUEUE_SIZE 10

// I/O exchange buffer for LoRa/MQTT Data
typedef struct {
  uint8_t port;
  uint16_t size;
  char raw[PAYLOAD_SIZE];
} payload_buffer;

// exchange buffers -- use MODBUS_CONFIGS macro....
static payload_buffer pd;
static cppQueue q(sizeof(payload_buffer), QUEUE_SIZE, FIFO);

/* OTAA params */
String uuid, mac;
uint8_t devEui[8];
uint8_t appEui[8];
uint8_t appKey[16];
uint8_t claimKey[16];

/* ABP para -- not used but MUST be declared anyway */
uint32_t devAddr;
uint8_t nwkSKey[16];
uint8_t appSKey[16];

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

// LoRaWan Parameters from Arduino IDE
bool keepNet                  = LORAWAN_NET_RESERVE;
bool loraWanAdr               = LORAWAN_ADR;
bool isTxConfirmed            = LORAWAN_UPLINKMODE;
bool overTheAirActivation     = LORAWAN_NETMODE;
DeviceClass_t  loraWanClass   = LORAWAN_CLASS;

// Other hard-coded parameters
uint32_t appTxDutyCycle       = MIN_DUTYCYCLE;
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_EU868;

/*!
  Number of trials to transmit the frame, if the LoRaMAC layer did not
  receive an acknowledgment. The MAC performs a datarate adaptation,
  according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
  to the following table:

  Transmission nb | Data Rate
  ----------------|-----------
  1 (first)       | DR
  2               | DR
  3               | max(DR-1,0)
  4               | max(DR-1,0)
  5               | max(DR-2,0)
  6               | max(DR-2,0)
  7               | max(DR-3,0)
  8               | max(DR-3,0)

  Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
  the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/

uint8_t appPort           = 0;
uint8_t confirmedNbTrials = 4;

static char*getHexString(const uint8_t* data, size_t length) {
  static char hexString[33]; // 16 bytes * 2 chars/byte + null terminator
  for (size_t i = 0; i < length && i < 16; i++) {
    sprintf(hexString + i * 2, "%02X", data[i]);
  }
  hexString[length * 2] = '\0'; // Null-terminate the string
  return hexString;
}

// cllsend flag
// LoraWan.send() must be called to flush/receive data.
// this flag tries to minime calls as they eat energy....

// Generates Device LoraWan OTAA Keys from chip Serial
static uint64_t chipID = getID() << 16;
static void mkDevKeys() {
  uint64_t chipID = getID() << 16;
  uint8_t dk = 0x70, ak1 = 0x81, ak2 = 0x83, ck1 = 0x91, ck2 = 0x93, ak2offs = sizeof(devEui);
  memcpy(devEui, &chipID, sizeof(devEui));
  for (uint8_t i = 0; i < sizeof(devEui); i++) {
    appEui[i] = devEui[i] ^ dk >> i;
    appKey[i] = devEui[i] ^ ak1 >> i;
    appKey[i + ak2offs] = devEui[i] ^ ak2 >> i;
    claimKey[i] = devEui[i] ^ ck1 >> i;
    claimKey[i + ak2offs] = devEui[i] ^ ck2 >> i;
  }
}

// downlink data handle function -- compiles inBuffer for nowireRecv
void __attribute__((weak)) downLinkDataHandle(McpsIndication_t *mcpsIndication)  {

  // cipy in buffer and cleanup
  memset(&pd, 0, sizeof(pd));
  memcpy(pd.raw, mcpsIndication->Buffer, mcpsIndication->BufferSize);

  // remove '\n' and '\r' chars -- consider only first vallid line....
  for (char *p = pd.raw; *p; p++)
    if (*p == '\n' || *p == '\r') {
      *p = 0;
      break;
    }

  // handle incoming data (only RPC admitted...)
  switch (mcpsIndication->Port) {
    case FPORT_RPC:
      //rpcManage(pd.raw, true);
      break;
    case FPORT_RPC_ASYNC:
      //rpcManage(pd.raw, false);
      break;
  }

  // cleanup buffer
  memset(&pd, 0, sizeof(pd));

}


// intialize LoRa Network Stuff
void mqttInit() {

  // Need 3.3V on Vext
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);

  ESP_LOGI(TAG, "Initializing Board Mcu...");
  boardInitMcu();
  ESP_LOGI(TAG, "Generating Device Keys...");
  mkDevKeys();
  ESP_LOGI(TAG, "Chip ID: %s", getHexString((uint8_t*)&chipID+2, sizeof(chipID)-2));
  ESP_LOGI(TAG, "Dev EUI: %s", getHexString(devEui, sizeof(devEui)));
  ESP_LOGI(TAG, "App EUI: %s", getHexString(appEui, sizeof(appEui)));
  ESP_LOGI(TAG, "App Key: %s", getHexString(appKey, sizeof(appKey))); 
  innerWdtEnable(true);

  // Various cleanup
  appDataSize = 0;
  memset(&pd, 0, sizeof(pd));

  deviceState = DEVICE_STATE_INIT;
  ESP_LOGI(TAG, "Initialization complete, entering main loop...");
  LoRaWAN.ifskipjoin();
}


// outgoing -- compiles payload to send at next run...
static void mqttUp(int port) {

  // cleanup
  memset(&pd, 0, sizeof(pd));
  const char *b=jsonGetBase64();
  uint16_t l=strlen(b);
  if (!l) return;
  if (l > PAYLOAD_SIZE) {
    ESP_LOGI(TAG, "Base64 Size of %d exceedes %d", l, PAYLOAD_SIZE);
    jsonInit();
    jsonAddObject("LoRaErr");
    jsonAddObject("message", "Data too big");
    jsonCloseAll();
    return; 
    }

  pd.port = port;
  pd.size = l;
  memcpy(pd.raw, b, l);

  ESP_LOGI(TAG, "Pushing Message: %s", jsonGetBuffer());
  ESP_LOGI(TAG, "Port=%d Size=%d CSize=%d", pd.port, jsonGetBufferSize(), l);

  // Push data i queue
  q.push(&pd);

  // cleanup buffer
  memset(&pd, 0, sizeof(pd));
}

// default 'up' to FPORT_STD
void mqttUp() {
  //mqttUp(FPORT_STD);
}

// handle for generic string event
static void mqttUp(int port, const char *object, const char *text) {
  jsonInit();
  jsonAddObject(object, text);
  jsonCloseAll();
  mqttUp(port);
}

// handle for generic string event
static void mqttUp(int port, const char *object, uint32_t value) {
  jsonInit();
  jsonAddObject(object, value);
  jsonCloseAll();
  mqttUp(port);
}

// default 'rpc' to FPORT_RPC (response)
static bool sync=false;
void mqttRpcUp(String rpcid) {
  uint8_t outport = FPORT_RPC;
  if (!sync) {
    jsonInit();
    jsonAddObject("ack");
    jsonAddObject("rpcid", rpcid.c_str());
    jsonCloseAll();
    outport = FPORT_EVT;
  }

  // Sends out and FORCES SEND
  mqttUp(outport);
  deviceState = DEVICE_STATE_SEND;
}

// Do nothing
bool netCheck() {
    return true;
}

bool netInit() {
  return true;
}

// Simulates Downlink and send buffers
bool mqttPoll() {

  // checks queue couint
  static uint32_t lastFeed;
  unsigned qCount = q.getCount();

  // feed watchdog
  if (millis() - lastFeed > 2000) {
    feedInnerWdt();
    lastFeed = millis();
  }

  // Check join status....
  // (!IsLoRaMacNetworkJoined) errorPixel();

  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:

      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:

      // Show device hard-coded tokens
      ESP_LOGI(TAG, "Claim: %s", claimKey, true);
      ESP_LOGI(TAG, "JToken: %s:%s:%s", getHexString(devEui, 8), getHexString(appEui, 8), getHexString(appKey, 16));

      // queue cfg request to launch after join
      mqttUp(FPORT_CFG, "CFG", millis());

      // Joins network
      LoRaWAN.join();

      break;

    case DEVICE_STATE_SEND:

      // Launch ONLY IF there are some data to send
      if (qCount) {

        // Debug
        ESP_LOGI(TAG, "Pulling and Sending Message 1/%d", qCount);

        // pop data
        q.pop(&pd);
        appPort = pd.port;
        appDataSize = pd.size;
        memcpy(appData, pd.raw, appDataSize);

        // Sends data -- may be a confirmation....
        LoRaWAN.send();

        // Buffers Cleanup
        appDataSize = 0;
        memset(&pd, 0, sizeof(pd));

      }
      else {
        // Sends data -- may be a confirmation or a flush
        //LoRaWAN.send();

      }

      //SendFrame();
      deviceState = DEVICE_STATE_CYCLE;
      break;

    case DEVICE_STATE_CYCLE:

      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle +  randr( 0, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;

    case DEVICE_STATE_SLEEP:

      LoRaWAN.sleep();
      break;

    default:

      deviceState = DEVICE_STATE_INIT;
      break;
  }
}

#endif
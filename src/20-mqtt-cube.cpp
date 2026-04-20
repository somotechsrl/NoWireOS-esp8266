#include "NowireOS.h"
#ifdef __CUBECELL__

#include <LoRaWan_APP.h>
#include <cppQueue.h>
#include "innerWdt.h"

#define DBG_LORA F("LoRaWan")

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
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

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

// cllsend flag
// LoraWan.send() must be called to flush/receive data.
// this flag tries to minime calls as they eat energy....

// Generates Device LoraWan OTAA Keys from chip Serial
void mkDevKeys() {
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

  // Activity
  commInPixel();

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
      rpcManage(pd.raw, true);
      break;
    case FPORT_RPC_ASYNC:
      rpcManage(pd.raw, false);
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

  // Messages -- useful for first registering
  //char *sep="-----------------------------------------------------------------------------------------";
  mkDevKeys();

  // init board
  boardInitMcu( );
  innerWdtEnable(true);

  // Various cleanup
  appDataSize = 0;
  memset(&pd, 0, sizeof(pd));

  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();

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

// outgoing -- compiles payload to send at next run...
static void mqttUp(int port) {

  // cleanup
  memset(&pd, 0, sizeof(pd));

  // stores size
  if (!(pd.size = jsonGetCompressedSize())) return;
  if (pd.size > PAYLOAD_SIZE) {
    debug(DBG_LORA, "Binary Size of " + String(pd.size) + " exceedes " + String(PAYLOAD_SIZE));
    jsonInit();
    jsonAddObject("LoRaErr");
    jsonAddObject("message", "Data too big");
    jsonCloseAll();
  }

  pd.port = port;
  memcpy(pd.raw, jsonGetEncryptedBuffer(), pd.size);

  int jsonsize = jsonGetBufferSize();
  debug(DBG_LORA, "Pushing Message: " + String(jsonGetBuffer()));
  debug(DBG_LORA, String("Port=") + pd.port + " Size=" + jsonsize + " CSize=" + pd.size);
  jsonClear();

  // Push data i queue
  q.push(&pd);

  // cleanup buffer
  memset(&pd, 0, sizeof(pd));
}

// default 'up' to FPORT_STD
void mqttUp() {
  mqttUp(FPORT_STD);
}

// default 'rpc' to FPORT_RPC (response)
void mqttRpcUp(String rpcid, bool sync) {
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

// default 'debug' to FPORT_RPC (response)
void mqttDebugUp(const char *json) {
  // doesn't do anything -- too muche traffic...
}

// Gets Pin Modes as character (Pullout,Input,Output)
const char getPinMode(uint8_t pin)
{
  return 'U';
}

// Do nothing
bool netCheck() {
    return true;
}

void checkReboot() {
  if (scheduleReboot && millis() - lastRebootCheck > scheduleReboot)
    HW_Reset(0);
}

static uint32_t lastFeed;

// Simulates Downlink and send buffers
void mqttPoll()
{

  // checks queue couint
  unsigned qCount = q.getCount();

  // feed watchdog
  if (millis() - lastFeed > 2000) {
    feedInnerWdt();
    lastFeed = millis();
  }

  // Check join status....
  if (!IsLoRaMacNetworkJoined) errorPixel();

  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:

      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:

      // Show device hard-coded tokens
      debug("Claim", AUTOHEX(claimKey, true));
      debug("JToken", AUTOHEX(devEui, false) + ":" + AUTOHEX(appEui, false) + ":" + AUTOHEX(appKey, false));

      // queue cfg request to launch after join
      mqttUp(FPORT_CFG, "CFG", millis());

      // Joins network
      LoRaWAN.join();

      break;

    case DEVICE_STATE_SEND:

      // Launch ONLY IF there are some data to send
      if (qCount) {

        // Activity
        commOutPixel();

        // Debug
        debug(DBG_LORA, String("Pulling and Sending Message 1/") + qCount);

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

// System Info
void sysGetInfo(void) {
  jsonAddObject("hw", String(BOARDID));
  jsonAddObject("fw", REVISION);
  jsonAddObject("EUI", hexKey(devEui, sizeof(devEui)));
}

// System Status
void sysGetStatus(void) {

  // Reads Battery Level
  const uint8_t batlevel = BoardGetBatteryLevel();
  jsonAddObject("batval", getBatteryVoltage());
  jsonAddObject("batlev", batlevel);
  jsonAddObject("batperc", (uint8_t)(100 * batlevel / 254));
  jsonAddObject("uptime", millis() / 1000);
}

// sends GPIO data
void sendStatus(void) {

  // do nothing....
  if (!configGetBit("STATUS"))
    return;

  jsonInit();
  jsonAddObject("status");
  sysGetStatus();
  jsonCloseAll();

  mqttUp();
}


#endif
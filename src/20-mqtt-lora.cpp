#ifdef CUBE_CELL
#include "00-main.h"
#include "HAL.h"
#include "cppQueue.h"
#include "10-encoder.h"
#include "20-mqtt-lora.h"

#define TAG "LORA"

// Standard Function port
#define FPORT_CFG 10
#define FPORT_EVT 11
#define FPORT_STD 20
#define FPORT_RPC 21
#define FPORT_RPC_ASYNC 22
#define MIN_DUTYCYCLE 15000
#define PAYLOAD_SIZE 128
#define QUEUE_SIZE 10

/* OTAA */
String uuid, mac;
uint8_t claimKey[16];
uint8_t devEui[8];
uint8_t appEui[8];
uint8_t appKey[16];
uint8_t appPort=1;

#define PAYLOAD_SIZE 128
#define DEBUG_SERIAL_ENABLED 0
/* Data transmission duty cycle.  value in [ms].*/
#define DEFAULT_DUTY_CYCLE_MINUTES MINUTES_1_IN_MILLISECONDS

static uint64_t chipID = getID() << 16;

// Will eliminate LualtekCubecell dependency in favor of direct calls to LoRaWAN stack -- see mqttPoll() for details
uint32_t appTxDutyCycle;
bool overTheAirActivation = true;
bool loraWanAdr = true;
bool keepNet = false;
bool isTxConfirmed = false;
uint8_t confirmedNbTrials = 4;
uint16_t userChannelsMask[6] = { 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };
DeviceClass_t loraWanClass = CLASS_A;
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_EU868;

/* ABP (not used) here as placeholder as required for Cubecell lib */
uint8_t nwkSKey[] = { 0x00 };
uint8_t appSKey[] = { 0x00 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

static char*getHexString(const uint8_t* data, size_t length) {
  static char hexString[33]; // 16 bytes * 2 chars/byte + null terminator
  for (size_t i = 0; i < length && i < 16; i++) {
    sprintf(hexString + i * 2, "%02X", data[i]);
  }
  hexString[length * 2] = '\0'; // Null-terminate the string
  return hexString;
}

// Generates Device LoraWan OTAA Keys from chip Serial
static void mkDevKeys() {
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

void onDownlinkReceived(McpsIndication_t *mcpsIndication) {

  if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK) {
    return;
  }

  if (mcpsIndication->RxData != true) {
    return;
  }

  /*
  switch(mcpsIndication->Port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      //handleChangeDutyCycle(mcpsIndication->Buffer[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      deviceState = DEVICE_STATE_INIT;
      break;
    default:
      break;
  }
  */
  
}
  
void downLinkDataHandle(McpsIndication_t *m) {

  char payload[256];
  onDownlinkReceived(m);
  ESP_LOGI(TAG, "Downlink received on port %d with payload size %d bytes", m->Port, m->BufferSize);
  snprintf(payload, sizeof(payload), "%s", (char*)m->Buffer);
  if (m->BufferSize > 0) {
    ESP_LOGI(TAG, "Downlink port=%d, payload: %s", m->Port, payload);
    }
    m->BufferSize = 0; // Clear the buffer after processing
  deviceState = DEVICE_STATE_SEND;
  }

// I/O exchange buffer for LoRa/MQTT Data
typedef struct {
  uint8_t port;
  uint16_t size;
  char raw[PAYLOAD_SIZE];
} payload_buffer;
// exchange buffers -- use MODBUS_CONFIGS macro....
static uint8_t qcount=0;
static payload_buffer pd;
static cppQueue q(sizeof(payload_buffer), QUEUE_SIZE, FIFO);

static void onSendUplink(int port) { 
  
  ESP_LOGI(TAG, "Uplink sent on ignored port %d (queue %d)", port, qcount);

  // Launch ONLY IF there are some data to send
  if (qcount > 0) {

    // Debug
    ESP_LOGI(TAG, "Uplink sent on port %d", port);

    // pop data
    q.pop(&pd);
    appPort = pd.port;
    appDataSize = pd.size;
    memcpy(appData, pd.raw, appDataSize);

    // Sends data -- may be a confirmation....
    ESP_LOGI(TAG, "Sending uplink on port %d with payload size %d bytes", appPort, appDataSize);
    LoRaWAN.send();

    // Buffers Cleanup
    appDataSize = 0;
    memset(&pd, 0, sizeof(pd));
    qcount--;
    }

  deviceState = DEVICE_STATE_CYCLE;
  }

void netInit() {
  
  delay(2000); // Wait for the system to stabilize

  ESP_LOGI(TAG, "Initializing Board Mcu...");
  boardInitMcu();

  ESP_LOGI(TAG, "Generating Device Keys...");
  mkDevKeys();
  ESP_LOGI(TAG, "Chip ID: %s", getHexString((uint8_t*)&chipID+2, sizeof(chipID)-2));
  ESP_LOGI(TAG, "Dev EUI: %s", getHexString(devEui, sizeof(devEui)));
  ESP_LOGI(TAG, "App EUI: %s", getHexString(appEui, sizeof(appEui)));
  ESP_LOGI(TAG, "App Key: %s", getHexString(appKey, sizeof(appKey))); 
  
  ESP_LOGI(TAG, "Initializing LoRaWAN stack...");
  //ll.setup();
  //ll.onSendUplink(onSendUplink);

  ESP_LOGI(TAG, "Joining LoRaWAN network...");
  }

void mqttInit() {
  //ll.onDownlink(downLinkDataHandle);
  //ll.onSend(onSendUplink);
  }

bool netCheck() {
  // In LoRaWAN, the device is always "connected" after joining the network, so we can return true here. The actual communication status will be handled in the mqttUp function and the downlink handler.
  return true;
}

bool mqttPoll() {
  //ESP_LOGI(TAG, "Polling LoRaWAN stack...");
  switch(deviceState) {
    case DEVICE_STATE_INIT: {
      #if (!CUSTOM_DEVEUI)
        LoRaWAN.generateDeveuiByChipID();
        printDevParam();
      #endif
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
      }
    case DEVICE_STATE_JOIN: {
      LoRaWAN.join();
      break;
      }
    case DEVICE_STATE_SEND: {
      onSendUplink(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
      }
    case DEVICE_STATE_CYCLE: {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
      }
    case DEVICE_STATE_SLEEP: {
      LoRaWAN.sleep();
      break;
      }
    default: {
      deviceState = DEVICE_STATE_INIT;
      break;
     }
    }
  return true;
  }

// outgoing -- compiles payload to send at next run...
static void mqttUp(int port) {

  // cleanup
  memset(&pd, 0, sizeof(pd));

  // Payload is stored a jsonbuffer (b64) from caller -- see mqttUp(port) for details
  char *b64 = (char *)jsonGetBase64();
  uint16_t length = strlen(b64);
  if (length > PAYLOAD_SIZE) {
    ESP_LOGE(TAG, "Binary Size of %d exceedes %d", length, PAYLOAD_SIZE);
    jsonInit();
    jsonAddObject("LoRaErr");
    jsonAddObject("message", "Data too big");
    jsonCloseAll();
    // recalcuate message....
    b64=(char *)jsonGetBase64();
    length=strlen(b64);
  }

  qcount++;
  pd.port = port;
  pd.size=length;
  memcpy(pd.raw, b64, pd.size);

  int jsonsize = jsonGetBufferSize();
  ESP_LOGI(TAG, "Prepared payload for port %d with size %d bytes", pd.port, pd.size);
  
  // Push data in queue -- see mqttPoll() for processing
  q.push(&pd);

  // buffer cleanuo
  memset(&pd, 0, sizeof(pd));
}

// default 'up' to FPORT_STD
// uses compiled json (b64) from caller -- see mqttUp(port) for details
void mqttUp() {
  mqttUp(FPORT_STD);
}
  
// default 'up' to FPORT_RPC
// uses compiled json (b64) from caller -- see mqttUp(port) for details
void mqttRpcUp(String responseID) {
  mqttUp(FPORT_RPC_ASYNC);
  }

#endif
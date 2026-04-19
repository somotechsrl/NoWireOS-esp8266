#ifdef CUBE_CELL

#include "00-main.h"
#include "HAL.h"

#define TAG "MTTN"

/* OTAA */
String uuid, mac;
uint8_t claimKey[16];
uint8_t devEui[8];
uint8_t appEui[8];
uint8_t appKey[16];
uint8_t appPort=1; // default application port for uplink, can be adjusted as needed for different applications or use cases


#define DEBUG_SERIAL_ENABLED 1
/* Data transmission duty cycle.  value in [ms].*/
#define DEFAULT_DUTY_CYCLE_MINUTES MINUTES_20_IN_MILLISECONDS

static uint64_t chipID = getID() << 16;
LualtekCubecell ll(CLASS_A, LORAMAC_REGION_EU868, MINUTES_20_COMMAND_INDEX);

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

void downLinkDataHandle(McpsIndication_t *mcpsIndication) {
  ll.onDownlinkReceived(mcpsIndication);
  deviceState = DEVICE_STATE_SEND;
}

void netInit() {
  ESP_LOGI(TAG, "Chip ID: %04x%08x", (uint16_t)(chipID >> 32), (uint32_t)chipID);
  ESP_LOGI(TAG, "Initializing Board Mcu...");
  boardInitMcu();
  ESP_LOGI(TAG, "Generating Device Keys...");
  mkDevKeys();
  ESP_LOGI(TAG, "Initializing LoRaWAN stack...");
  ll.setup();
  }

void mqttInit() {
  ESP_LOGI(TAG, "Joining LoRaWAN network...");
  ll.join();
 }

bool netCheck() {
  // In LoRaWAN, the device is always "connected" after joining the network, so we can return true here. The actual communication status will be handled in the mqttUp function and the downlink handler.
  return true;
}

bool mqttPoll() {
  ll.loop();
  return true;
}

void mqttUp() {
  appPort=10; // default application port for uplink, can be adjusted as needed for different applications or use cases
  ESP_LOGW(TAG,"Not Yet Implemented");
  }

void mqttRpcUp(String responseID) {
  appPort=20; // default application port for RPC responses, can be adjusted as needed for different applications or use cases
  ESP_LOGW(TAG,"Not Yet Implemented");
  ESP_LOGI(TAG, "Publishing RPC response with ID: %s",responseID.c_str());
  ESP_LOGI(TAG, "RPC response payload size: %d bytes", appDataSize);
  ESP_LOGW(TAG, "Not Yet Implemented!!!");
  }


#endif
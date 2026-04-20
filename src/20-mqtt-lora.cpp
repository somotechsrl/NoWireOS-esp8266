#ifdef CUBE_CELL_OFF

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
#define DEFAULT_DUTY_CYCLE_MINUTES MINUTES_1_IN_MILLISECONDS

static uint64_t chipID = getID() << 16;
LualtekCubecell ll(CLASS_A, LORAMAC_REGION_EU868, MINUTES_20_COMMAND_INDEX);

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

void downLinkDataHandle(McpsIndication_t *m) {
  char payload[256];
  ll.onDownlinkReceived(m);
  ESP_LOGI(TAG, "Downlink received on port %d with payload size %d bytes", m->Port, m->BufferSize);
  snprintf(payload, sizeof(payload), "%s", (char*)m->Buffer);
  if (m->BufferSize > 0) {
    ESP_LOGI(TAG, "Downlink port=%d, payload: %s", m->Port, payload);
    }
    m->BufferSize = 0; // Clear the buffer after processing
  deviceState = DEVICE_STATE_SEND;
  }

void onSendUplink(uint8_t port) {
  ESP_LOGI(TAG, "Uplink sent on port %d", port);
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
  }

void mqttInit() {
  ESP_LOGI(TAG, "Initializing LoRaWAN stack...");
  ll.setup();
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
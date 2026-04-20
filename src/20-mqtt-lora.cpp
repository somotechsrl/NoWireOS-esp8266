#ifdef CUBE_CELL

#include <Arduino.h>
#include <LoRaWan_APP.h>

// LoRa Parameters
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
LoRaMacRegion_t loraRegion = ACTIVE_REGION;
DeviceClass_t loraClass = CLASS_A;
uint32_t appTxDutyCycle = 15000;
bool overTheAirActivation = true;
bool lorawanAdrOn = true;
bool isTxConfirmed = false;
uint8_t appPort = 10;
uint8_t confirmedNbTrials = 4;

static char*getHexString(const uint8_t* data, size_t length) {
  static char hexString[33]; // 16 bytes * 2 chars/byte + null terminator
  for (size_t i = 0; i < length && i < 16; i++) {
    sprintf(hexString + i * 2, "%02X", data[i]);
  }
  hexString[length * 2] = '\0'; // Null-terminate the string
  return hexString;
}

static uint64_t chipID = getID() << 16;
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

static void prepareTxFrame(uint8_t port) {
    appDataSize = 2;
    appData[0] = 0x00;
    appData[1] = 0x01;
}

void netInit() {
    Serial.begin(115200);
    BoardInitMcu();
    mkDevKeys();
    LoRaWAN.begin(loraClass, loraRegion);
    LoRaWAN.setDataRateTxPower(DR_0, TX_POWER_14);
    LoRaWAN.setAdaptiveDataRate(lorawanAdrOn);
}

void joinNetwork() {
    if (overTheAirActivation) {
        LoRaWAN.joinOTAA(appEui, devEui, appKey, userChannelsMask);
    } else {
        LoRaWAN.joinABP(devEui, appSKey, nwkSKey, userChannelsMask);
    }
}   

void onJoin() {
    Serial.println("Successfully joined LoRaWAN network");
    isTxConfirmed = true;
}

void mqtPoll() {
    if (isTxConfirmed == isTxDone) {
        prepareTxFrame(appPort);
        LoRaWAN.send();
        isTxDone = false;
    }
    radio.IrqProcess();
}

void mqttInit() {
    Serial.println("Initializing LoRaWAN stack...");
    joinNetwork();
}

void mqttPoll() {
    if (isTxConfirmed == isTxDone) {
        prepareTxFrame(appPort);
        LoRaWAN.send();
        isTxDone = false;
    }
    radio.IrqProcess();
}

void mqttUp() {
    // In LoRaWAN, the device is always "connected" after joining the network, so we can return true here. The actual communication status will be handled in the mqttPoll function and the downlink handler.
    return;
}   

void mqttRpcUp(String responseID) {
    // LoRaWAN is not designed for request-response patterns, so this function can be used to prepare the next uplink message with the responseID included in the payload or as part of the application data. The actual sending will be handled in the mqttPoll function.
    return;
}

#endif
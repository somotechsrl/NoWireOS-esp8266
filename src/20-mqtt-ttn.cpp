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

#define DEBUG_SERIAL_ENABLED 1
/* Data transmission duty cycle.  value in [ms].*/
#define DEFAULT_DUTY_CYCLE_MINUTES MINUTES_20_IN_MILLISECONDS
/* Application port (BME280) */
uint8_t appPort = 3;

#define debugSerial Serial

LualtekCubecell ll(CLASS_A, LORAMAC_REGION_EU868, MINUTES_20_COMMAND_INDEX);
// BME280 bme280;

int temperature, humidity, batteryVoltage, batteryLevel;
long pressure;

// Generates Device LoraWan OTAA Keys from chip Serial
void mkDevKeys() {
  uuid=mac="" + getID();
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

void downLinkDataHandle(McpsIndication_t *mcpsIndication) {
  ll.onDownlinkReceived(mcpsIndication);
  deviceState = DEVICE_STATE_SEND;
}

/* Prepares the payload of the frame */
void mqttUp(uint8_t port) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(500);

  // while (!bme280.init()) {
  //   if (DEBUG_SERIAL_ENABLED) {
  //     debugSerial.println("Waiting for device...");
  //   }
  // }

  // temperature = bme280.getTemperature();
  // humidity = bme280.getHumidity();
  // pressure = bme280.getPressure();

  // Turn the power to the sensor off again
  Wire.end();
  digitalWrite(Vext, HIGH);

  batteryVoltage = getBatteryVoltage();
  batteryLevel = (BoardGetBatteryLevel() / 254) * 100;

  appDataSize = 12;
  appData[0] = highByte(temperature);
  appData[1] = lowByte(temperature);

  appData[2] = highByte(humidity);
  appData[3] = lowByte(humidity);

  appData[4] = (byte) ((pressure & 0xFF000000) >> 24 );
  appData[5] = (byte) ((pressure & 0x00FF0000) >> 16 );
  appData[6] = (byte) ((pressure & 0x0000FF00) >> 8  );
  appData[7] = (byte) ((pressure & 0X000000FF)       );

  appData[8] = highByte(batteryVoltage);
  appData[9] = lowByte(batteryVoltage);

  appData[10] = highByte(batteryLevel);
  appData[11] = lowByte(batteryLevel);

  if (DEBUG_SERIAL_ENABLED) {
    debugSerial.print("Temperature: ");
    debugSerial.print(temperature);
    debugSerial.print("C, Humidity: ");
    debugSerial.print(humidity);
    debugSerial.print("%, Pressure: ");
    debugSerial.print(pressure / 100);
    debugSerial.print(" mbar, Battery Voltage: ");
    debugSerial.print(batteryVoltage);
    debugSerial.print(" mV, Battery Level: ");
    debugSerial.print(batteryLevel);
    debugSerial.println(" %");
  }
}

void mqttUp() {
  // NOt yet implemented send Buffer64 to standard LoRa Port
  ESP_LOGW(TAG,"Not Yet Implemented");
  }

void mqttRpcUp(String responseID) {
    ESP_LOGI(TAG, "Publishing RPC response with ID: %s",responseID.c_str());
    ESP_LOGI(TAG, "RPC response payload size: %d bytes", appDataSize);
    ESP_LOGW(TAG, "Not Yet Implemented!!!");
  }

void netInit() {
  ESP_LOGI(TAG, "Initializing LoRaWAN network...");
  mkDevKeys();
  ll.setup();
  ll.join();
}

void mqttInit() {
  if (DEBUG_SERIAL_ENABLED) {
    ESP_LOGI(TAG, "Initializing serial debug...");
    ESP_LOGI(TAG, "Serial debug started at 9600 baud");
  }

  boardInitMcu();
  mkDevKeys();

  ll.setup();
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

#endif
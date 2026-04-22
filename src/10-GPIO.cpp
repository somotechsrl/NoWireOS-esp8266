#include "main.h"
#include "10-encoder.h"
#include "20-rpc-utils.h"

#define TAG "GPIO"
#ifdef HAS_GPIO


// DIO and AIO maps and configs
static uint8_t adc[] = ANALOGS, dio[] = DIGITAL;

// return formatted json string from gpio
static void gpioGetJsonData() {
    
    char name[BUFTINY]; 

    // calculates json string ase registers
    for (uint8_t pin = 0; pin < sizeof(dio); pin++) {
        snprintf(name, sizeof(name), "D%02d", dio[pin]);
        jsonAddObject(name,(uint8_t)digitalRead(dio[pin]));
        }
    for (uint8_t pin = 0; pin < sizeof(adc); pin++) {
        snprintf(name, sizeof(name), "A%02d", adc[pin]);
        jsonAddObject(name,(uint16_t)analogRead(adc[pin]));
        }
}   

// return formatted json string from gpio
// digiatl pins are returned as bitwise int
void gpioJsonModes() {
    
  // calculates json string ase registers
  jsonAddArray("DIO");
  for (uint8_t pin = 0; pin < sizeof(dio); pin++) {
    jsonAddValue((uint8_t)digitalRead(dio[pin]));
  }
  jsonClose();

  jsonAddArray("ADC");
  for (uint8_t pin = 0; pin < sizeof(adc); pin++) {
    jsonAddValue((uint16_t)analogRead(adc[pin]));
  }
  jsonClose();
}

// Returns gpio configuration
void gpioConfig() {

  // calculates json string ase registers
  jsonAddArray("DIO");
  for (uint8_t pin = 0; pin < sizeof(dio); pin++) {
    jsonAddValue(dio[pin]);
  }
  jsonClose();
  jsonAddArray("ADC");
  for (uint8_t pin = 0; pin < sizeof(adc); pin++) {
    jsonAddValue(adc[pin]);
  }
  jsonClose();
}

// gets digital pin internal number
// starts from 1 to DIGITAL configured
uint8_t getDigitalPin(uint8_t sequence) {
  return sequence >= sizeof(dio) ? 0 : dio[sequence];
}

// gets analog pin internal number
// starts from 1 to ANALOGS configured
uint8_t getAnalogPin(uint8_t sequence) {
  return sequence >= sizeof(adc) ? 0 : adc[sequence];
}

// maps analog/digital pin in Dxx Axx format
uint8_t getPinNumber(String pinid) {
  // pin must be in Axxx or Dxxx format
  if (pinid.substring(0, 1) == "A") return getAnalogPin(pinid.substring(1).toInt());
  if (pinid.substring(0, 1) == "D") return getDigitalPin(pinid.substring(1).toInt());
  return 0;
}

// maps analog/digital pin in Dxx Axx format
uint8_t getPinUsage(String pinid) {
  // pin must be in Axxx or Dxxx format
  //  if(pinid.substring(0,1)=="A") return getPinMode(getAnalogPin(pinid.substring(1).toInt()));
  //  if(pinid.substring(0,1)=="D") return getPinMode(getDigitalPin(pinid.substring(1).toInt()));
  return 0;
}

// sends GPIO data
void gpioMasterTask() {

    // check if Dataloggin is remotely enabled
    if(!rpcIsEnabled("gpio")) {
        ESP_LOGW(TAG, "GPIO Master Task is disabled, skipping execution");
        return;
        }

    jsonInit();
    // init json block for new server, if same server as previous call, will aggregate into same block
    jsonAddObject("SEQ",(uint8_t)0);
    jsonAddObject("DEV","gpio");
    jsonAddObject("BUS","local");
    jsonAddObject("DRV","gpio");
    jsonAddObject("data");
    gpioGetJsonData();
    jsonCloseAll();
    mqttUp();
    ESP_LOGI(TAG, "Completed GPIO Master Task");
}

#else
void gpioMasterTask() {
    ESP_LOGW(TAG, "GPIO functionality is not available on this platform");
    } 
#endif

#ifdef RELAY_PIN

void relayOn() {
    // turn on relay with given id, can be extended to support multiple relays and different types of relays as needed for more complex control of external devices
    digitalWrite(#RELAY_PIN, HIGH);
    }

void relayOff() {
    // turn off relay with given id, can be extended to support multiple relays and different types of relays as needed for more complex control of external devices
    digitalWrite(#RELAY_PIN, LOW);
    } 

#else
void relayOn(uint8_t relay_id) {
    ESP_LOGW(TAG, "Relay functionality is not available on this platform");
    } 
void relayOff(uint8_t relay_id) {
    ESP_LOGW(TAG, "Relay functionality is not available on this platform");
    }
#endif
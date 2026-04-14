#include "main.h"
#include "10-encoder.h"
#include "20-rpc-utils.h"
#include "20-mqtt.h"

// DIO and AIO maps and configs
static uint8_t adc[] = ANALOGS, dio[] = DIGITAL;

// return formatted json string from gpio
// digiatl pins are returned as bitwise int
void gpioJsonStatus() {

  uint32_t digital = 0;

  // Digital
  jsonAddArray("DIO");
  for (uint8_t pin = 0; pin < sizeof(dio); pin++) {
    jsonAddValue(digitalRead(dio[pin]));
    digital |= digitalRead(dio[pin]) << pin;
  }
  jsonClose();

  // Analogs
  jsonAddArray("ADC");
  for (uint8_t pin = 0; pin < sizeof(adc); pin++) {
    jsonAddValue((uint16_t)analogRead(adc[pin]));
  }
  jsonClose();

}

// return formatted json string from gpio
// digiatl pins are returned as bitwise int
void gpioJsonModes() {
    
  uint32_t digital = 0;

  // calculates json string ase registers
  jsonAddArray("DIO");
  for (uint8_t pin = 0; pin < sizeof(dio); pin++) {
    jsonAddValue(digitalRead(dio[pin]));
    digital |= digitalRead(dio[pin]) << pin;
  }
  jsonClose();

  jsonAddArray("ADC");
  for (uint8_t pin = 0; pin < sizeof(adc); pin++) {
    jsonAddValue(analogRead(adc[pin]));
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
  if (rpcIsEnabled("gpio")) {
    jsonInit();
    gpioJsonStatus();
    jsonCloseAll();
    mqttUp();
  }
}
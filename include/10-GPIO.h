#ifndef __PROTO_10_GPIO_CPP__
#define __PROTO_10_GPIO_CPP__
//Extracted Prototyes
// ****************************
// src/10-GPIO.cpp prototypes
// ****************************
void gpioJsonModes();
void gpioConfig();
uint8_t getDigitalPin(uint8_t sequence);
uint8_t getAnalogPin(uint8_t sequence);
uint8_t getPinNumber(String pinid);
uint8_t getPinUsage(String pinid);
void gpioMasterTask();
void gpioMasterTask();
void relayOn();
void relayOff();
void relayOn(uint8_t relay_id);
void relayOff(uint8_t relay_id);
#endif

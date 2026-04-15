#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>
#include "HAL.h"


// for ssl client, can be extended to include certificate handling as needed for more secure communication with MQTT broker or other services
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <EEPROM.h>
#include "HAL.h" 
#include "00-utils.h"

extern bool provisionMode;
#define RESET_BUTTON_PIN 3

extern String uuid, mac;
extern uint64_t systimestep;
extern uint64_t mbutimestep;
extern bool led_blink_enabled;

#endif

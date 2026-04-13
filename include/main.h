#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>
#include "HAL.h"

// Wifi stuff platform specific
#ifdef ESP32
#include "WiFi.h"
#include "WebServer.h"
#else
// port of ESP_LOGx to ESP8266
#include "00-debug.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#endif

// for ssl client, can be extended to include certificate handling as needed for more secure communication with MQTT broker or other services
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// Include other necessary headers for the project, can be extended as needed for additional functionality
#ifdef ESP32
void logger_mqtt();
void logger_serial();
void logger_default();
void logger_off();
#else
#include "00-debug.h"
#endif

#include <EEPROM.h>
#include "HAL.h" 
#include "00-utils.h"

extern bool provisionMode;
#define RESET_BUTTON_PIN 3

extern String uuid, mac;
extern uint64_t timestep;
extern bool led_blink_enabled;

#endif

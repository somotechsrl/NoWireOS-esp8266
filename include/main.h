#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>        
#include "HAL.h" 
#include "00-debug.h"

// Wifi common definitions
extern ESP8266WebServer server;
extern bool provisionMode;
#define RESET_BUTTON_PIN D3
typedef struct WiFiConfig {
    char ssid[32];
    char password[64];
} WiFiConfig;   
extern WiFiConfig wifiConfig;
extern char uuid[33], mac[18];
extern uint32_t timestep;

#endif

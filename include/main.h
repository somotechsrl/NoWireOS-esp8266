#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>

#ifdef ESP32
#include "WiFi.h"
#include "WebServer.h"
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "00-debug.h"
#endif

#include <EEPROM.h>
#include "HAL.h" 
#include "00-utils.h"

// Wifi common definitions
#ifdef ESP32
#include "WiFi.h"
#include "WebServer.h"
extern WebServer server;
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
extern ESP8266WebServer server;
#endif
extern bool provisionMode;
#define RESET_BUTTON_PIN 3
typedef struct WiFiConfig {
    char ssid[32];
    char password[64];
} WiFiConfig;   
extern WiFiConfig wifiConfig;
extern String uuid, mac;
extern uint64_t timestep;
extern bool led_blink_enabled;

#endif

#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>
#include "HAL.h"

#ifdef ESP32
#include "WiFi.h"
#include "WebServer.h"
extern WebServer server;
#else
#include "00-debug.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "00-debug.h"
extern ESP8266WebServer server;
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
#endif

#include <EEPROM.h>
#include "HAL.h" 
#include "00-utils.h"

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

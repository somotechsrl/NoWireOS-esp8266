#ifndef __PROTO_MAIN_CPP__
#define __PROTO_MAIN_CPP__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>         

typedef strunct WifiConfig {
    char ssid[32];
    char password[64];
} WifiConfig;   
extern WifiConfig wifiConfig;

#define RESET_BUTTON_PIN D3

#define BUFSIZE 512
#define BUFTINY 64  


#endif

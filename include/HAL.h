#ifndef __ZZZ_HAL_H__
#define __ZZZ_HAL_H__

// Simgle machine string, will be surpassed by auto enumaraton of machine type in future, for now can be used for logging and debugging purposes
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
#define BOARDID "d1mini"
#define ARCH "ESP8266"
#define USEWIFI 1
#endif
#ifdef ARDUINO_ESP8266_NODEMCU
#define BOARDID "nodemcu"
#define ARCH "ESP8266"  
#endif
#ifdef ARDUINO_ESP8266_NODEMCU_ESP12E
#define BOARDID "nodemcu"
#define ARCH "ESP8266"      
#endif
#ifdef ARDUINO_ESP32_WROOM_DA
#define BOARDID "esp32-dev"
#define ARCH "ESP32"
#endif
#ifdef ARDUINO_ESP32_DEV
#define BOARDID "esp32-dev"
#define ARCH "ESP32"
#endif
#ifdef ARDUINO_D1_MINI32
#define BOARDID "esp32-mini"
#define ARCH "ESP32"
#endif
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define BOARDID "esp32-s3"
#define ARCH "ESP32"
#endif
#ifdef CUBE_CELL
#define BOARDID "cubecell-board"
#define ARCH "CUBE_CELL"
#endif  

// Default debug modes
#ifndef MINTSTEP
#define MINTSTEP 300
#endif

// Define NTP server and timezone (e.g., CET/CEST)
#include <time.h>
#define NTP_SERVER "pool.ntp.org"
#define TZ "GMT +1"

// Include other necessary headers for the project, can be extended as needed for additional functionality
#ifdef ESP32
#include <Adafruit_NeoPixel.h>
// for ssl client, can be extended to include certificate handling as needed for more secure communication with MQTT broker or other services
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "20-mqtt-wifi.h"
// logger functions for log redirections
// esp32 has ESP_LOGx macros
void logger_mqtt();
void logger_serial();
void logger_default();
void logger_off();
// wifi and web server for provisioning
#include "WiFi.h"
#include "WebServer.h"
#include "esp_log.h"
// other macros
#define USEWIFI 1   
#define ARCH "ESP32"
#define BUFSIZE 2048
#define BUFTINY 512
#define MODBUS_CONFIGS 80 // maximum number of Modbus configurations, can be adjusted as needed
#define GPIO_WIFI_RESET 4
#define NEOPIXEL_PIN 16
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif

#ifdef ESP8266
#include "00-debug.h"
#include <Adafruit_NeoPixel.h>
// wifi and web server for provisioning
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// for ssl client, can be extended to include certificate handling as needed for more secure communication with MQTT broker or other services
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "20-mqtt-wifi.h"
// other macros
#define USEWIFI 1
#define ARCH "ESP8266"
#define BUFSIZE 1536
#define BUFTINY 256
#define MODBUS_CONFIGS 80 // maximum number of Modbus configurations, can be adjusted as needed
#define GPIO_WIFI_RESET D3
#define NEOPIXEL_PIN D2
#define ONBOARD_LED LED_BUILTIN
#define DIGITAL  {D0,D1,D2,D4,D8}
#define ANALOGS  {A0}
#endif

#ifdef CUBE_CELL
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "00-debug.h"
#include "LoRaWan_APP.h"
#include "20-mqtt-lora.h"
//#define USEWIFI 1
#define ARCH "CUBE_CELL"
#define SERIAL_SPEED 9600
#define BUFSIZE 256
#define BUFTINY 128
#define MODBUS_CONFIGS 1 // maximum number of Modbus configurations, can be adjusted as needed
//#define GPIO_WIFI_RESET D3
#define NEOPIXEL_PIN 2
#define ONBOARD_LED 2
#define DIGITAL  {6,7,8,16,30,33,34}
#define ANALOGS  {2}
#endif

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200 
#endif

#endif
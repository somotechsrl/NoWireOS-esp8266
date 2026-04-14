#ifndef __ZZZ_HAL_H__
#define __ZZZ_HAL_H__

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
#define ARCH "ESP32"
#define BUFSIZE 2048
#define BUFTINY 512
#define MODBUS_CONFIGS 80 // maximum number of Modbus configurations, can be adjusted as needed
#define GPIO_WIFI_RESET 4
#define NEOPIXEL_PIN 16
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#else
#include "00-debug.h"
// wifi and web server for provisioning
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// other macros
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


// Simgle machine string, will be surpassed by auto enumaraton of machine type in future, for now can be used for logging and debugging purposes
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
#define BOARDID "d1mini"

#endif
#ifdef ARDUINO_ESP8266_NODEMCU
#define BOARDID "nodemcu"
#endif
#ifdef ARDUINO_ESP8266_NODEMCU_ESP12E
#define BOARDID "nodemcu"
#endif
#ifdef ARDUINO_ESP32_WROOM_DA
#define BOARDID "esp32-dev"
#endif
#ifdef ARDUINO_ESP32_DEV
#define BOARDID "esp32-dev"
#endif
#ifdef ARDUINO_D1_MINI32
#define BOARDID "esp32-mini"
#endif
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define BOARDID "esp32-s3-devkitc-1"

#endif

#endif
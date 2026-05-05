#ifndef __ZZZ_HAL_H__
#define __ZZZ_HAL_H__

// Simgle machine string, will be surpassed by auto enumaraton of machine type in future, for now can be used for logging and debugging purposes
#ifdef ARDUINO_ESP8266_ESP01
#define BOARDID "esp01"
#define ARCH "ESP8266"
#define ONBOARD_LED 2
#undef LED_BUILTIN
#define LED_BUILTIN 2
#define RELAY_PIN 0
#define MODBUS_TCP
#define USEWIFI 1
#endif
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
#define BOARDID "d1mini"
#define ARCH "ESP8266"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define DIGITAL  {D0,D1,D2,D4,D8}
#define ANALOGS  {A0}
#define USEWIFI 1
#define ONBOARD_LED D1
#endif
#ifdef ARDUINO_ESP8266_NODEMCU
#define BOARDID "nodemcu"
#define ARCH "ESP8266"  
#define HAS_GPIO
#define MODBUS_RTU
#define DIGITAL  {D0,D1,D2,D4,D8}
#define ANALOGS  {A0}
#define ONBOARD_LED LED_BUILTIN
#endif
#ifdef ARDUINO_ESP8266_NODEMCU_ESP12E
#define BOARDID "nodemcu"
#define ARCH "ESP8266"      
#define HAS_GPIO
#define MODBUS_RTU
#define DIGITAL  {D0,D1,D2,D4,D8}
#define ANALOGS  {A0}
#define ONBOARD_LED LED_BUILTIN
#endif
#ifdef ARDUINO_ESP32_WROOM_DA
#define BOARDID "esp32-dev"
#define ARCH "ESP32"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif
#ifdef ARDUINO_ESP32_DEV
#define BOARDID "esp32-dev"
#define ARCH "ESP32"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif
#ifdef ARDUINO_D1_MINI32
#define BOARDID "esp32-mini"
#define ARCH "ESP32"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define BOARDID "esp32-s3"
#define ARCH "ESP32"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define USEWIFI 1
#define ONBOARD_LED 2
#define DIGITAL  {2,13,14,15,18,19,21,22,23,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif
#ifdef CUBE_CELL
#define BOARDID "cubecell-board"
#define ARCH "CUBE_CELL"
#define MODBUS_RTU
#endif  
#ifdef HELTEC_WIFI_LORA_32
#define BOARDID "heltec-lora32"
#define ARCH "ESP32"
#define HAS_GPIO
#define MODBUS_RTU
#define MODBUS_TCP
#define ONBOARD_LED 25
#define DIGITAL  {0,2,4,5,12,13,14,15,16,17,18,19,21,22,23,26,27,32,33,34,35,36,39}
#define ANALOGS  {A0,A3,A4,A5,A6,A7}
#endif

// Default debug modes
#ifndef MINTSTEP
#define MINTSTEP 300
#endif

// Include other necessary headers for the project, can be extended as needed for additional functionality
#ifdef ESP32
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
#define HAS_OTA
#define MODBUS_CONFIGS 80 // maximum number of Modbus configurations, can be adjusted as needed
#endif

#ifdef ESP8266
#include "00-debug.h"
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
#endif

#ifdef CUBE_CELL
#define LoRaWAN_DEBUG_LEVEL 0
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "00-debug.h"
#define ARCH "CUBE_CELL"
#define SERIAL_SPEED 9600
#define BUFSIZE 256 
#define BUFTINY 128
#define MODBUS_CONFIGS 2 // maximum number of Modbus configurations, can be adjusted as needed
#define NEOPIXEL_PIN 2
#define ONBOARD_LED 2
#define DIGITAL  {6,7,8,16,30,33,34}
#define ANALOGS  {2}
#define SSERIAL_PINS 14,15
#include "softSerial.h"
#endif

//Uses LoraWan instead of Wifi+MQTT
#ifdef LORAWAN_CLIENT
#include "LoRaWan_APP.h"
#include "20-mqtt-lora.h"
#endif

#ifndef SSERIAL_PINS
#define SSERIAL_PINS D5,D6
#endif

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200 
#endif

#endif
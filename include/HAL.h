#ifndef __ZZZ_HAL_H__
#define __ZZZ_HAL_H__

// Default debug modes
#define DBG_SPEED 115200L
#define DBG_MODES "8N1"

// D1 MINI
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
#define __MQTT__
#define __ESPWIFI__
#define __MODBUS_TCP__
#define MODBUS_CONFIGS 20
#define GPIO_WIFI_RESET D3
#define DIGITAL  {D0,D1,D2,D4,D8}
#define ANALOGS  {A0}
#define ARCH "ESP8266"
#define BOARDID F("d1mini")
#define NEOPIXEL_PIN D2
#define ONBOARD_LED LED_BUILTIN
#endif

// NodeMCU12E
#ifdef ARDUINO_ESP8266_NODEMCU_ESP12E
#define __MQTT__
#define __ESPWIFI__
#define __MODBUS_TCP__
#define MODBUS_CONFIGS 20
#define GPIO_WIFI_RESET D3 // Flash Button
#define ARCH "ESP8266"
#define BOARDID "nodemcu"
#define NEOPIXEL_PIN D2
#define ONBOARD_LED LED_BUILTIN
#endif

// ESP32_WROOM_32
#ifdef ARDUINO_ESP32_WROOM_DA
#define __MQTT__
#define __ESPWIFI__
#define __MODBUS_TCP__
#define __ESP32__
#define MODBUS_CONFIGS 80
#define BUFSIZE 4096
#define GPIO_WIFI_RESET 4
#define ARCH "ESP32"
#define BOARDID esp32-dev
#define NEOPIXEL_PIN 16
#define ONBOARD_LED 
#endif// ESP32_DEV

#ifdef ARDUINO_ESP32_DEV
#define __MQTT__
#define __ESPWIFI__
#define __MODBUS_TCP__
#define __ESP32__
#define MODBUS_CONFIGS 80
#define BUFSIZE 4096
#define GPIO_WIFI_RESET 4
#define ARCH "ESP32"
#define BOARDID "esp32-dev"
#define NEOPIXEL_PIN 16
#define ONBOARD_LED 2
#endif

// MINI_32
#ifdef ARDUINO_D1_MINI32
#define __MQTT__
#define __ESPWIFI__
#define __MODBUS_TCP__
#define __ESP32__
#define MODBUS_CONFIGS 80
#define GPIO_WIFI_RESET 4
#define ARCH "ESP32"
#define BOARDID "esp32-mini"
#define NEOPIXEL_PIN 16
#define ONBOARD_LED LED_BUILTIN
#endif

    
// Default buffer size
#ifndef BUFSIZE
#define BUFSIZE 1024
#endif

#ifndef BUFTINY
#define BUFTINY 512
#endif

#ifndef MINTSTEP
#define MINTSTEP 300
#endif

// Default Serial data lines
#define DEBUG_SERIAL Serial

#endif
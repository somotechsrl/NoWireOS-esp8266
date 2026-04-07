#include "NowireOS.h"
#include <stdio.h>

// Common usage iobuffer
static String sbuffer;
char iobuffer[BUFSIZE];


/*
// some modules libraries use prinf for debug, so we redefine it
// redefines printf as it's used in some libraris...
int printf(const char *format, ...) {
  int res;
  va_list vl;
  va_start(vl, format);
  res = vsprintf(iobuffer, format, vl);
  DEBUG_SERIAL.print(iobuffer);
  return res;
}
*/

// debug status fag
enum debugModes { debugOff, debugOn, DebugNet };
static uint8_t debugMode = debugOn;
uint32_t schedulereboot=0;

// generic error json
void jsonError(const char *id,const char *msg) {
  jsonAddObject(id);
  jsonAddObject("ERROR", msg);
}

// generic error json
void jsonError(const char *msg) {
  jsonAddObject("ERROR", msg);
}

// Not implememented json object
void jsonErrorNotImplemented() {
  jsonError("Not Implemented");

}

void jsonErrorNoData() {
  jsonError("No Data");

}
void jsonErrorNoConfig() {
  jsonError("Configuration Unavailable");

}
void jsonErrorConfigLimits() {
  jsonError("Configuration Limits Reached");

}
void jsonErrorConfigAlreadySet() {
  jsonError("Configuration Already Set");

}

// hex dump key in char
static char *hexKey(uint8_t *key, uint8_t ksize,bool upper=false) {
  // max key is 64 bytes
  char hbuf[3];
  memset(iobuffer, 0, sizeof(iobuffer));
  for (uint8_t i = 0; i < ksize; i++) {
    sprintf(hbuf, upper ? "%02X" : "%02x", *key++);
    strcat(iobuffer, hbuf);
  }
  return iobuffer;
}

// Wrapper to reboot
// reboot management/scheduler
uint32_t scheduleReboot=0,lastRebootCheck;

// schedule/deschedule Reboot
void enableReboot() {
  scheduleReboot = REBOOTDELAY;
  lastRebootCheck = millis();
  jsonAddObject("message", String("Rebooting in ") + scheduleReboot / 1000 + "s");
}

void cancelReboot() {
  jsonAddObject("message",scheduleReboot ? "Reboot Cancelled" : "No reboot scheduled");
  scheduleReboot=0;
}

// enable/disable alternate Debug channel
void setDebugMode(uint8_t dMode) {
  const char *r;
  if (dMode > DebugNet) {
    return;
  }
  jsonAddObject("DebugMode", dMode);
  debugMode = dMode;

}

// Debug output (if DEBUG) defined!!
void debug(const char *t, const char *d) {
  debug(String(t), String(d));
}
void debug(String t, String d) {
  switch (debugMode) {
    case DebugNet:
      mqttDebugUp((t + ": " + d).c_str());
      break;
    case debugOff:
      break;
    default:
    case debugOn:
      DEBUG_SERIAL.println(t + ": " + d);
      break;

  }
}
void debug(String d) {
  debug("DEBUG", d);
}

// Flags for enable/disable LEDs
static bool ledEnabled=true;

void initPixel() {
// May be used together!
#ifdef ONBOARD_LED
pinMode(ONBOARD_LED,OUTPUT);
#endif
}

void ledEnable() {
  ledEnabled=true;
}
void ledDisable() {
  ledEnabled=false;
}

void showPixel(int r, int g, int b) {
  // do nothing if led disabled
#ifdef ONBOARD_LED
#ifdef ESP8266
  digitalWrite(ONBOARD_LED,r || g || b ? LOW : HIGH);
#endif
#ifdef ESP32
  digitalWrite(ONBOARD_LED,r || g || b ? HIGH : LOW);
#endif
#endif
}

void blinkPixel(int r, int g, int b) {
  if(!ledEnabled) return;
  showPixel(r,g,b);
  delay(BLINKDELAY);
  showPixel(0,0,0);
  delay(BLINKDELAY);
}

// Activty flashing
static uint32_t lastBlink=0;
void alivePixel() {
    if(millis() - lastBlink > 1000) {
      blinkPixel(0, 0, NEOPIXEL_BRI);
      lastBlink=millis();
    }
}

// Datalogger
void datalogPixel() {
    showPixel(0, NEOPIXEL_BRI, 0);
}

// Comm Out Flashing
void commOutPixel() {
    blinkPixel(NEOPIXEL_BRI, NEOPIXEL_BRI,0);
}

// Comm In Flashing
void commInPixel() {
    blinkPixel(0,NEOPIXEL_BRI,NEOPIXEL_BRI);
}

// WIFI config
void wifiConfigPixel() {
  showPixel(NEOPIXEL_BRI,NEOPIXEL_BRI,0);
}

// DataSend Flashing
void errorPixel() {
    showPixel(NEOPIXEL_BRI,0,0);
}

// DataSend Flashing
void identifyPixel() {
    showPixel(NEOPIXEL_BRI,NEOPIXEL_BRI,NEOPIXEL_BRI);
    delay(2000);
    showPixel(0,0,0);
    jsonAddObject("message","System Identified");
}

#include "NowireOS.h"
#ifdef __ESPWIFI__

#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// Wifi Manager
static WiFiManager wifiManager;

// the debounce time; increase if the output flickers
static char apName[128];
static String mac;

static const int DEBOUNCE_DELAY = 4000;
static int lastSteadyState = LOW;       // the previous steady state from the input pin
static int lastFlickerableState = LOW;  // the previous flickerable state from the input pin
static int currentState;                // the current reading from the input pin

static unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

void netInit() {

  WiFi.begin();
  uuid = mac = WiFi.macAddress();
  debug("UUID:", uuid);
  uuid.replace(":", "");
  debug("UUID:", uuid);
  String macStr = "nwos-" + String(uuid);
  macStr.toCharArray(apName, sizeof(apName));
  debug("WiFiAP", apName);

  WiFi.hostname(apName);
  wifiManager.setConfigPortalBlocking(true);
  wifiManager.autoConnect(apName);

}

// Must be caled in loop -- need 'Wifi reset button'
void netCheck() {

  wifiManager.process();

  // Variables will change:
  // the following variables are unsigned longs because the time, measured in
  // milliseconds, will quickly become a bigger number than can be stored in an int.

  // Reset WIFI Button setup
  currentState = digitalRead(GPIO_WIFI_RESET);

  // If the switch/button changed, due to noise or pressing:
  if (currentState != lastFlickerableState) {
    lastDebounceTime = millis();
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (lastSteadyState == HIGH && currentState == LOW) {
      debug("WIFI", "Reset Requested -- >4s");
      WiFi.disconnect();
      wifiConfigPixel();
      wifiManager.startConfigPortal(apName);
    }
    else if (lastSteadyState == LOW && currentState == HIGH) {
      debug("WIFI", "Button Released -- no action");
    }
    lastSteadyState = currentState;
  }

  delay(100);
}

// Gets Pin Modes as character (Pullout,Input,Output)
const char getPinMode(uint8_t pin)
{
  uint32_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint32_t *reg = portModeRegister(port);
  //if (*reg & bit) return (OUTPUT);
  if (*reg & bit) return 'O';

  volatile uint32_t *out = portOutputRegister(port);
  //return ((*out & bit) ? INPUT_PULLUP : INPUT);
  return ((*out & bit) ? 'P' : 'I');
}


void checkReboot() {
  if (scheduleReboot && millis() - lastRebootCheck > scheduleReboot)
    ESP.restart();
}

#ifdef __ESP32__
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif
#endif

// Compiles data for GetInfo and Status
// Don't intialize object!!!!
void sysGetInfo(void) {

  char ipbuf[20];
  IPAddress ip = WiFi.localIP();
  sprintf(ipbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  jsonAddObject("hw", String(BOARDID).c_str());
  jsonAddObject("sn", String(uuid).c_str());
  jsonAddObject("fw", REVISION);
  jsonAddObject("mac", mac.c_str());
  jsonAddObject("up", (uint32_t)(millis() / 1000));
  jsonAddObject("ip", ipbuf);
#ifdef __ESP32__
  String temp = String((temprature_sens_read() - 32) / 1.8);
  jsonAddObject("te", temp.c_str());
  jsonAddObject("mf",ESP.getFreeHeap());
#endif

}

// System Status (alias of info)
void sysGetStatus(void) {
  sysGetInfo();
}

// System Info (status)
void sendStatus(void) {

  // do nothing....
  if (!configGetBit("STATUS"))
    return;

  jsonInit();
  jsonAddObject(String(BOARDID).c_str());
  //sysGetInfo();
  sysGetStatus();
  jsonCloseAll();
  mqttUp();
}

#endif

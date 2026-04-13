#include "main.h"
#include "10-encoder.h"
#include "20-mqtt.h"
#include "20-rpc-utils.h"
#include "revision.h"
#include "HAL.h"

static int systemEnabled=false;

void rpcEnable(const char *key) {
  systemEnabled=true;
  ESP_LOGI(TAG, "System Enabled");
  jsonAddObject("result","System Enabled");
  } 

void rpcDisable(const char *key) {
  systemEnabled=false;
  ESP_LOGI(TAG, "System Disabled");
  jsonAddObject("result","System Disabled");
  } 
  
void sysGetInfo(void) {

  char ipbuf[20],rutbuf[20];
  IPAddress ip = WiFi.localIP();
  sprintf(ipbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  ip = WiFi.gatewayIP();
  sprintf(rutbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  float uptime=(millis() / 1000.0f);


  // Perform calculations
  int totalSeconds = (int)uptime;
  int days = totalSeconds / 86400;
  int hours = (totalSeconds % 86400) / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;

  // Format as string (optional)
  char buffer[30];
  sprintf(buffer, "%02d days, %02d:%02d:%02d", days, hours, minutes, seconds);

  jsonAddObject("ar", ARCH);
  jsonAddObject("hw", BOARDID);
  jsonAddObject("sn", uuid.c_str());
  jsonAddObject("mac",mac.c_str());
  jsonAddObject("fw", REVISION);
  jsonAddObject("us", uptime);
  jsonAddObject("ut", buffer);
  jsonAddObject("ip", ipbuf);
  jsonAddObject("gw", rutbuf);
  jsonAddObject("heap", ESP.getFreeHeap());
#ifdef __ESP32__
  String temp = String(temperatureRead());
  jsonAddObject("te", temp.c_str());
  jsonAddObject("mf",ESP.getFreeHeap());
#endif

}


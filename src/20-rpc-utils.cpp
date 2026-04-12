#include "main.h"
#include "10-encoder.h"
#include "20-mqtt.h"
#include "20-rpc-utils.h"
#include "revision.h"

void sysGetInfo(void) {

  char ipbuf[20];
  IPAddress ip = WiFi.localIP();
  sprintf(ipbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  float uptime=(millis() / 1000.0f);

  jsonAddObject("ar", ARCH);
  jsonAddObject("hw", BOARDID);
  jsonAddObject("sn", uuid.c_str());
  jsonAddObject("mac",mac.c_str());
  jsonAddObject("fw", REVISION);
  jsonAddObject("up", uptime);
  jsonAddObject("ip", ipbuf);
#ifdef __ESP32__
  String temp = String((temprature_sens_read() - 32) / 1.8);
  jsonAddObject("te", temp.c_str());
  jsonAddObject("mf",ESP.getFreeHeap());
#endif

}


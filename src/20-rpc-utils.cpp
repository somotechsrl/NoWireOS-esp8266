#include "main.h"
#include "10-encoder.h"
#include "20-mqtt.h"
#include "20-rpc-utils.h"
#include "revision.h"
#include "HAL.h"

#define TAG "RPCU"
static int modbusEnabled=false,gpioEnable=false;

void Enable(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    modbusEnabled=true;
    ESP_LOGI(TAG, "Modbus Enabled");
    jsonAddObject("result","Modbus Enabled");
    return;
  }
  if(strcmp(type, "gpio") == 0) {
    gpioEnable=true;
    ESP_LOGI(TAG, "RPC Enabled");
    jsonAddObject("result","RPC Enabled");
    return;
  }
  
  ESP_LOGW(TAG, "Unknown type for Enable: %s", type);
  jsonAddObject("result","ERROR: Unknown type for Enable: %s", type);
  }
}

void rpcDisable(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    modbusEnabled=false;
    ESP_LOGI(TAG, "Modbus Disabled");
    jsonAddObject("result","Modbus Disabled");
    return;
    }
  if(strcmp(type, "gpio") == 0) {
    gpioEnable=false;
    ESP_LOGI(TAG, "GPIO Disabled");
    jsonAddObject("result","GPIO Disabled");
    return;
    }
  ESP_LOGW(TAG, "Unknown type for Enable: %s", type);
  jsonAddObject("result","ERROR: Unknown type for Enable: %s", type);  
  } 
  
bool rpcIsEnabled(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    return modbusEnabled;
  }
  if(strcmp(type, "gpio") == 0) {
    return gpioEnable;
  }
  ESP_LOGW(TAG, "Unknown type for IsEnabled: %s", type);
  return false;
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


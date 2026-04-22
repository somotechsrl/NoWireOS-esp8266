#include "main.h"
#include "10-encoder.h"
#include "20-rpc-utils.h"
#include "revision.h"
#include "HAL.h"

#define TAG "RPCU"
static int modbusEnabled=false,gpioEnable=false,sysInfoEnable=false;
 
void rpcEnable(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    modbusEnabled=true;
    ESP_LOGI(TAG, "Modbus Enabled");
    jsonAddObject("result","Modbus Enabled");
    return;
  }
  if(strcmp(type, "gpio") == 0) {
    gpioEnable=true;
    ESP_LOGI(TAG, "GPIO Enabled");
    jsonAddObject("result","GPIO Enabled");
    return;
  }
  if(strcmp(type, "sysinfo") == 0) {
    sysInfoEnable=true;
    ESP_LOGI(TAG, "Sysinfo Enabled");
    jsonAddObject("result","Sysinfo Enabled");
    return;
  }
  
  ESP_LOGW(TAG, "Unknown type for Enable: %s", type);
  jsonAddObject("result","ERROR: Unknown type for Enable: %s", type);
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
  if(strcmp(type, "sysinfo") == 0) {
    sysInfoEnable=false;
    ESP_LOGI(TAG, "Sysinfo Disabled");
    jsonAddObject("result","Sysinfo Disabled");
    return;
    }
  ESP_LOGW(TAG, "Unknown type for Enable: %s", type);
  jsonAddObject("result","ERROR: Unknown type for Enable: %s", type);  
  } 

void rpcEnabled(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    ESP_LOGI(TAG, "Modbus Enabled: %s", modbusEnabled ? "true" : "false");
    jsonAddObject("result",modbusEnabled ? "true" : "false");
    return;
    }
  if(strcmp(type, "gpio") == 0) {
    ESP_LOGI(TAG, "GPIO Enabled: %s", gpioEnable ? "true" : "false");
    jsonAddObject("result",gpioEnable ? "true" : "false");
    return;
    }
  if(strcmp(type, "sysinfo") == 0) {
    ESP_LOGI(TAG, "Sysinfo Enabled: %s", sysInfoEnable ? "true" : "false");
    jsonAddObject("result",sysInfoEnable ? "true" : "false");
    return;
    } 
  if(strcmp(type, "all") == 0) {
    ESP_LOGI(TAG, "Modbus Enabled: %s", modbusEnabled ? "true" : "false");
    ESP_LOGI(TAG, "GPIO Enabled: %s", gpioEnable ? "true" : "false");
    ESP_LOGI(TAG, "Sysinfo Enabled: %s", sysInfoEnable ? "true" : "false");
    jsonAddObject("modbus", modbusEnabled ? "true" : "false");
    jsonAddObject("gpio", gpioEnable ? "true" : "false");
    jsonAddObject("sysinfo", sysInfoEnable ? "true" : "false");
    return;
    }
  ESP_LOGW(TAG, "Unknown type for Enabled: %s", type);
  jsonAddObject("result","ERROR: Unknown type for Enabled: %s", type);  
  }

bool rpcIsEnabled(const char *type) {
  if(strcmp(type, "modbus") == 0) {
    return modbusEnabled;
  }
  if(strcmp(type, "gpio") == 0) {
    return gpioEnable;
  }
  if(strcmp(type, "sysinfo") == 0) {
    return sysInfoEnable;
  } 
  ESP_LOGW(TAG, "Unknown type for IsEnabled: %s", type);
  return false;
}

void sysGetInfo(void) {

#ifndef CUBE_CELL
  char ipbuf[20],rutbuf[20];
  IPAddress ip = WiFi.localIP();
  sprintf(ipbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  ip = WiFi.gatewayIP();
  sprintf(rutbuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#endif

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

  jsonAddObject("sn", uuid.c_str());
  jsonAddObject("us", uptime);
  jsonAddObject("fw", REVISION);
#ifndef CUBE_CELL
  jsonAddObject("ar", ARCH);
  jsonAddObject("hw", BOARDID);
  jsonAddObject("ut", buffer);
  jsonAddObject("mac",mac.c_str());
  jsonAddObject("ip", ipbuf);
  jsonAddObject("gw", rutbuf);
  jsonAddObject("heap", ESP.getFreeHeap());
#endif
#ifdef __ESP32__
  String temp = String(temperatureRead());
  jsonAddObject("te", temp.c_str());
  jsonAddObject("mf",ESP.getFreeHeap());
#endif

}

void sysGetInfoTask() {
    if(!rpcIsEnabled("sysinfo")) {
        ESP_LOGW(TAG, "Sysinfo Task is disabled, skipping execution");
        return;
        }
    jsonInit(); 
    jsonAddObject("DEV","sysinfo"); 
    jsonAddObject("BUS","internal");
    jsonAddObject("DRV","sysinfo"); 
    jsonAddObject("data");
    sysGetInfo();
    jsonCloseAll();
    mqttUp();
    ESP_LOGI(TAG, "System info sent successfully");
    } 

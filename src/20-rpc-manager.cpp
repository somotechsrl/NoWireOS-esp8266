#include "main.h"
#include <string.h>
#include "30-rpc-cmd.h"
#include "30-rpc-ids.h"
#include "10-encoder.h"
#include "10-modbus-tcp.h"
#include "20-rpc-utils.h"
#include "20-modbus-master.h"
#include "10-wifi-provision.h"
#include "time.h"

#define TAG "RPC"
#define WIFIDISCONNECT_DELAY 10000 // delay before disconnecting WiFi as per RPC command, can be adjusted as needed for more immediate disconnection or longer delay for graceful shutdowns in real-world applications
bool trigger = false;
uint64_t mbutimestep=10000; // Modbus timestep
uint64_t systimestep=300000; // System info timestep

// retrives command sequence if for switch/case
static int getCommandID(const char *rpcmd) {
  int size=sizeof(RPC_cmd)/sizeof(RPC_cmd[0]);
  for (int i = 0; i<size;i++) {
    if (!strncmp(rpcmd, RPC_cmd[i], strlen(RPC_cmd[i]))) return i;
  }
  return -1;
}

// Work with string which is more efficent
static char *p,rpcb[BUFTINY];
void rpcManage(const char *payload, bool sync) {

  bool wifiDisconnect=false; // flag to handle WiFi disconnection as per RPC command, can be expanded later to include more complex connection management as needed for robustness in real-world applications
  ESP_LOGI(TAG, "Received: %s", payload);

  // extracts ID and Command
  char *st;
  strcpy(rpcb, payload);

  char *request_id = (char *)((p=strtok_r(rpcb,"|",&st))!=NULL ? p : "");
  char *rpccommand = (char *)((p=strtok_r(NULL,"|",&st))!=NULL ? p : "");
  char *rpc_params = (char *)((p=strtok_r(NULL,"|",&st))!=NULL ? p : "");

  char respid[BUFTINY];
  snprintf(respid, sizeof(respid), "R%s", request_id);
  ESP_LOGI(TAG, "Splitting: %s :: %s :: %s --> %s", request_id,rpccommand,rpc_params, respid);

  // sets rpcid
  int cmdid = getCommandID(rpccommand);
  ESP_LOGI(TAG, "ID: %s Command: %s (ID: %d)", respid, rpccommand, cmdid);

  // recived something.. init json Response Buffer
  // submodule MUST NOT call jsonInit/jsonCloseAll for respnses!!!
  jsonInit();
  jsonAddObject(respid);
  jsonAddObject("result");
  
  // rpcStatus default is 'OK'
  char *rpcStatus = (char *)"OK";
  char result[BUFSIZE] = "Executed successfully";

  switch (cmdid) {
    case CFG_Debug:
      //setDebugMode(bitname.toInt());
      break;
    case CFG_Leds_Enable:
      led_blink_enabled = true;
      jsonAddObject("value","HB LED Enabled");
      break;
    case CFG_Leds_Disable:
      led_blink_enabled = false;
      jsonAddObject("value","HB LED Disabled");
      break;
    case CFG_Modbus_AddCall:
      addModbusAggregatedCall(rpc_params);
      break;
    case CFG_Modbus_Timeout: 
      {
      int16_t mbto=atoi(rpc_params);
      if(mbto<100 || mbto>2000) {
        jsonAddObject("value","ERROR: Invalid Modbus timeout value, must be between 100 and 2000 ms");
        ESP_LOGE(TAG, "Invalid Modbus timeout value received: %u ms", mbto);
        break;
        }
      setModbusTimeout(mbto);
      jsonAddObject("value", getModbusTimeout());
      }
      break;
    case CFG_Timestep:
      // timestep is received in s, converted in ms
      if (*rpc_params) mbutimestep = atoi(rpc_params)*1000;
      jsonAddObject("value", (uint32_t)mbutimestep/1000);
      break;
    case CFG_LOG_Mqtt:
      logger_mqtt();
        jsonAddObject("value","MQTT Logger Enabled");
        break;
      case CFG_LOG_Local:
        logger_default();
        jsonAddObject("value","Local Logger Enabled");
        break;
      case CFG_LOG_Off:
      case CFG_LOG_None:
        logger_off();
        jsonAddObject("value","Logging Disabled");
        break;
      case RPC_Enable:
        rpcEnable(rpc_params);
        break;
      case RPC_Disable:
        rpcDisable(rpc_params);
        break;
      case RPC_Enabled:
        rpcIsEnabled(rpc_params);
        break;
      case RPC_Trigger:
        if(!strcmp(rpc_params,"modbus")) {
          jsonAddObject("value","OK: Modbus Triggered");
          //trigger_task_handle = modbus_master_task_handle;
          //trigger_modbus=true;
          }
      else {
        jsonAddObject("value","ERROR:Unknown Trigger");
        }
      break;
      
    case RPC_List:
      {
      jsonAddArray("RPC.List");
      int size=sizeof(RPC_cmd)/sizeof(RPC_cmd[0]);
      for (int i = 0; i<size;i++) {
        jsonAddValue((const char *)RPC_cmd[i]);
        }
      jsonClose();
      break;
      }
    case Sys_GetInfo:
      sysGetInfo();
      break;
    case Sys_GetStatus:
      sysGetInfo();
      break;
    case Sys_Reboot:
      //enableReboot();
      break;
    case Sys_Update_Cancel:
      //cancelUpdate(); 
      break;
    case Sys_Cancel_Reboot:
      //cancelReboot();
      break;
    case Sys_Identify:
      //identifyPixel();
      break;
    case Sys_WiFi_Disconnect:
      ESP_LOGI(TAG,"Disconnecting WiFi as per RPC command in %d ms", WIFIDISCONNECT_DELAY);
      jsonAddObject("Info","WiFi Disconnection Scheduled in %d s", WIFIDISCONNECT_DELAY/1000);
      wifiDisconnect=true;
      break;
  
    // ************ Unknow management
    default:
      rpcStatus = (char *)"KO";
      snprintf(result, sizeof(result), "%s(%s): %s", rpccommand,rpc_params, "not implemented");
      jsonAddObject("value",result);
 
  }

  // Closes response Buffer and sets response status
  jsonClose();
  jsonAddObject("status", rpcStatus);
  jsonCloseAll();

  ESP_LOGI(TAG, "MODE: %s", sync ? "SYNC:" : "ASYNC:");
  ESP_LOGI(TAG, "%s", jsonGetBuffer());
  if(sync) mqttRpcUp(respid);

  // Delayed actions based on RPC command, can be expanded later to include more complex actions as needed for robustness in real-world applications
  // allow to send response before performing actions that may disrupt connectivity or require a delay for graceful shutdowns, such as WiFi disconnection or system reboot, can be adjusted as needed for more immediate actions or longer delays for graceful shutdowns in real-world applications

  // discnnect Wifi after response is sent to ensure we can send the response before disconnecting, can be expanded later to include more complex connection management as needed for robustness in real-world applications
  if(wifiDisconnect) {
    delay(WIFIDISCONNECT_DELAY); // delay to ensure response is sent before disconnecting, can be adjusted as needed for performance optimization in real-world applications
    wifiReset(); // reset WiFi credentials and restart to ensure clean state, can be expanded later to include more graceful shutdown procedures as needed for robustness in real-world applications
    }
}

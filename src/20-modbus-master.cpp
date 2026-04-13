#include "main.h"
#include "10-encoder.h"
#include "10-modbus-tcp.h"
#include "20-mqtt.h"
#include "20-modbus-master.h"

// Modbus configuration entry
#define XTAG 32

typedef struct {
  uint16_t rs;
  uint8_t rn;
} cfg_call;

// Modbus Configuration block
typedef struct {
  char tag[XTAG];
  char ad[XTAG];
  uint8_t fn,ncalls;
  // max MODBUS_CONFIG  calls... -- see HAL.h
  cfg_call calls[MODBUS_CONFIGS];
} modbus_config;


#define TAG "MBMAS"
static int amodbus_cnt=0;
static char *amodbus_cfg[MODBUS_CONFIGS];

void addModbusAggregatedCall(char *params) {


  ESP_LOGI(TAG, "Adding Modbus call with params: %s", params);  
  jsonAddObject("CFG_STRING",params);

  // param is in the form dev_id;tpc:address:port:unit;function;r:n,r:n,r:n....
  // checks if we reached limit
  if(amodbus_cnt>=MODBUS_CONFIGS) {
      ESP_LOGE(TAG, "Maximum number of Modbus configurations reached: %d", MODBUS_CONFIGS);
    jsonAddObject("CFG_RESULT","ERROR: Maximum number of configs reached: %d",amodbus_cnt);
    return;
    }

  uint8_t fn;
  char tag[32],ad[32];
  char rs_str[BUFTINY];
  
  // separates root values tag,ad,fn,registers
   // typical format for aggregated call: tag;ad;fn;rs1:rn1,rs2:rn2,rs3:rn3,... where rs is starting register and rn is number of registers to read, allows for batch processing of multiple registers in one call for more efficient transmission and processing in modbus client task loop
  // splits single aggregated call with comma separated registers into multiple calls with same tag, ad, fn, but different rs and rn, then adds each call to config for processing in modbus client task loop 
  if (sscanf(params, "%31[^;];%31[^;];%hhu;%511[^;]s", tag, ad, &fn, rs_str) != 4) {
    ESP_LOGE(TAG, "Invalid Modbus call parameters: %s", params);
    jsonAddObject("CFG_RESULT","ERROR: Invalid params: %s", params);
    return; 
    }

  // check dups
  for(int i=0;i<amodbus_cnt;i++) {
    if(strcmp(amodbus_cfg[i],params) == 0) {
      ESP_LOGW(TAG, "Ignored (duplicate) Modbus configuration: %s", params);
      jsonAddObject("CFG_RESULT","Ignored (duplicate) configuration: %s", params);
      return;
    }
  }

  amodbus_cfg[amodbus_cnt] = strdup(params);
  if (amodbus_cfg[amodbus_cnt] == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for Modbus configuration: %s", params);
    jsonAddObject("CFG_RESULT","ERROR: Memory allocation failed for params: %s", params);
    return;
  }
  amodbus_cnt++;
  ESP_LOGI(TAG, "Added Modbus configuration: %s", params);
  jsonAddObject("CFG_RESULT","Added configuration: %s", params);
  }

static modbus_config *parse_modbus_cfg(char *params) {

  char *st,rs_str[BUFTINY];
  static modbus_config conf;
  memset(&conf,0,sizeof(conf));
  
  // separates root values tag,ad,fn,registers
   // typical format for aggregated call: tag;ad;fn;rs1:rn1,rs2:rn2,rs3:rn3,... where rs is starting register and rn is number of registers to read, allows for batch processing of multiple registers in one call for more efficient transmission and processing in modbus client task loop
  // splits single aggregated call with comma separated registers into multiple calls with same tag, ad, fn, but different rs and rn, then adds each call to config for processing in modbus client task loop 
  if (sscanf(params, "%31[^;];%31[^;];%hhu;%511[^;]s",conf.tag, conf.ad, &conf.fn, rs_str) != 4) {
    jsonAddObject("CFG_RESULT","ERROR: Invalid params: %s", params);
    return NULL; 
    }

  // explodes rs_str into individual register sets, separated by comma, in format rs:rn, then adds each call to config with same tag, ad, fn, but different rs and rn for each register set, allows for batch processing of multiple registers in one call for more efficient transmission and processing in modbus client task loop
  int i=0;
  char *token = strtok_r((char *)rs_str, ",",&st);
  while (token != NULL) {
    if (sscanf(token, "%hu:%hhu", &conf.calls[i].rs, &conf.calls[i].rn) == 2) {
      if(i>=MODBUS_CONFIGS) {
        ESP_LOGE(TAG, "Maximum number of calls per configuration reached: %d", MODBUS_CONFIGS);
        jsonAddObject("CFG_RESULT","ERROR: Maximum number of calls per config reached: %d", MODBUS_CONFIGS);
        break;
        }
      if(conf.calls[i].rn == 0) {
        ESP_LOGE(TAG, "Invalid number of registers to read (rn) in config: %s", token);
        jsonAddObject("CFG_RESULT","ERROR: Invalid number of registers to read (rn) in config: %s", token);
        continue;
        }
      i++;
    } else {
      jsonAddObject("CFG_RESULT","Invalid register set: %s", token);
      ESP_LOGE(TAG, "Invalid register set: %s", token);
    }
    token = strtok_r(NULL, ",",&st);
   }

   conf.ncalls=i;

return &conf;

}

// Modbus client task, will loop through calls in config and execute them, then send json result to mqtt
void modbusMasterTask() {

    // Needs MQTT to work
    if(!mqttPoll()) {
        ESP_LOGW(TAG, "MQTT not connected, skipping Modbus Master Task");
        return;
        }

    modbus_config *conf;

    static char server_type[32]; // rtu, tcp
    static char server_host[BUFTINY]; // serial,speed,sb,parity || server FQDN or IP
    static uint16_t server_port,server_unit_id; 

    ESP_LOGI(TAG, "Starting Modbus Master Task -- active configurations: %d", amodbus_cnt);

    // replace loop with connector loop RTU/TCP
    for(int i=0;i<amodbus_cnt;i++) {
      
      jsonInit();

      // explodes in modbus_cfg
      if((conf=parse_modbus_cfg(amodbus_cfg[i])) == NULL) {
        jsonAddObject("ERROR", "Failed to get modbus config");
        ESP_LOGE(TAG, "Failed to get modbus config for params: %s", amodbus_cfg[i]);
        continue;
        }

      // starta read time
      uint32_t startTime = millis();

      // init json block for new server, if same server as previous call, will aggregate into same block
      jsonAddObject("DEV",conf->tag);
      jsonAddObject("BUS",conf->ad);
      jsonAddObject("DRV","modbus");
      jsonAddObject("data");

      // extract modbus call parameters from call->ad 
      if(sscanf(conf->ad, "%31[^':']:%31[^':']:%hu:%hu", server_type, server_host, &server_port, &server_unit_id) != 4) {
          jsonAddObject("CFG_Error", "Failed to parse Modbus TCP call address: %s", conf->ad);
          ESP_LOGE(TAG, "Failed to parse Modbus TCP call address: %s", conf->ad);
          continue;
          }

      // TCP Network call
      if(strcmp(server_type, "tcp") == 0) {
          // pings server to check if it's reachable before attempting connection, can help avoid long timeouts in case of unreachable server
          if (modbusTcpConnect(server_host, server_port, server_unit_id)) {
            for(int i=0;i<conf->ncalls;i++) {
              // gest response buffer and sets respLength
              modbusTcpReadJson(server_unit_id,conf->fn, conf->calls[i].rs, conf->calls[i].rn); 
              // polls mqtt as we d't have a rtx system
              mqttPoll();
              }
            modbusTcpDisconnect();
          } else {
            ESP_LOGE(TAG, "Failed to connect to Modbus TCP server: %s:%d", server_host, server_port);
            jsonAddObject("ERROR", "Failed to connect to Modbus TCP server: %s:%d", server_host, server_port);
            } 
        }
      // RTU Serial call - not implemented yet, placeholder for future expansion 
      else if(strcmp(server_type, "rtu") == 0) {
          // RTU client call - not implemented yet, placeholder for future expansion
          ESP_LOGE(TAG, "RTU client call not implemented yet: %s", conf->ad);
          jsonAddObject("ERROR", "RTU client call not implemented yet: %s", conf->ad);
          continue; 
        } 
      // Unknown server type
      else {
          ESP_LOGE(TAG, "Unknown server type in Modbus call address: %s", conf->ad);
          jsonAddObject("ERROR", "Unknown server type: %s", server_type);
          continue;
        }

      jsonClose();

      //end read time and add to json response
      uint32_t endTime = millis();
      jsonAddObject("read_time_ms", endTime - startTime);
      ESP_LOGI(TAG, "Completed Modbus Master Task for config: %s, read time: %lu ms", conf->tag, endTime - startTime);

      // block opened in loop, should be called after processing all calls to ensure json is properly closed for mqtt transmission
      jsonCloseAll(); // ensure all blocks are closed, in case of config errors that may cause block structure issues
      mqttUp();
    } 

  }


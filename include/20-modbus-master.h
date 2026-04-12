#ifndef __PROTO_20_MODBUS_MASTER_CPP__
#define __PROTO_20_MODBUS_MASTER_CPP__
//Extracted Prototyes
// ****************************
// src/20-modbus-master.cpp prototypes
// ****************************
void addModbusAggregatedCall(char *params);
static modbus_config *parse_modbus_cfg(char *params);
static void modbus_master_task(void *pvParameters);
#endif

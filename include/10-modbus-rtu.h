#ifndef __PROTO_10_MODBUS_RTU_CPP__
#define __PROTO_10_MODBUS_RTU_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-rtu.cpp prototypes
// ****************************
uint16_t calculateCRC(uint8_t *data, uint8_t length);
void modbusRTUInit(uint32_t baudrate);
void modbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity);
void modbusRTUInit(uint32_t baudrate);
uint16_t *modbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity);
#endif

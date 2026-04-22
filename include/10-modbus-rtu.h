#ifndef __PROTO_10_MODBUS_RTU_CPP__
#define __PROTO_10_MODBUS_RTU_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-rtu.cpp prototypes
// ****************************
uint16_t calculateCRC(uint8_t *data, uint8_t length);
uint16_t *mdbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity);
#endif

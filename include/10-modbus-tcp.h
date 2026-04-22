#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
void setModbusTimeout(uint16_t timeout_ms);
uint16_t getModbusTimeout();
bool modbusTcpConnect(const char *host, int port, uint8_t unitId);
void modbusTcpDisconnect();
uint16_t *modbusTcpReadJson(uint8_t unit_id, uint8_t func, uint16_t start_address, uint16_t quantity);
bool modbusTcpConnect(const char *host, int port, uint8_t unitId);
void modbusTcpDisconnect();
uint16_t *modbusTcpReadJson(uint8_t unit_id, uint8_t func, uint16_t start_address, uint16_t quantity);
void setModbusTimeout(uint16_t timeout_ms);
uint16_t getModbusTimeout();
#endif

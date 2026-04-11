#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
bool modbusTcpConnect(const char *host, int port, uint8_t unitId);
void modbusTcpDisconnect();
uint16_t *modbusTcpReadRegisters(uint8_t function, uint16_t startAddr, uint16_t &quantity);
void readModbusTcp();
#endif

#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
void modbusTcpDisconnect();
bool modbusTcpReadRegisters(uint8_t function, uint16_t startAddr, uint16_t quantity, uint16_t* values);
void readModbusTcp();
#endif

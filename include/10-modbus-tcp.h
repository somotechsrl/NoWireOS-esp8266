#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
bool receiveResponse(uint8_t* response, uint16_t maxLength, uint16_t& length);
void readModbusTcp();
#endif

#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
static void sendRequest(uint8_t* request, uint16_t length);
static uint16_t receiveResponse(uint8_t* response, uint16_t maxLength);
bool modbusTcpConnect(const char *host, int port, uint8_t unitId);
void modbusTcpDisconnect();
static uint16_t *modbusTcpRead(uint8_t unitId,uint8_t function, uint16_t startAddr, uint16_t quantity);
static bool writeRegister(uint16_t addr, uint16_t value);
uint16_t *modbusTcpReadJson(uint8_t unit_id, uint8_t func, uint16_t start_address, uint16_t quantity);
void readModbusTcp();
#endif

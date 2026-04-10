#ifndef __PROTO_10_MODBUS_TCP_CPP__
#define __PROTO_10_MODBUS_TCP_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-tcp.cpp prototypes
// ****************************
    void sendRequest(uint8_t* request, uint16_t length);
    bool receiveResponse(uint8_t* response, uint16_t maxLength, uint16_t& length);
    bool connect();
    void disconnect();
    bool readRegisters(uint8_t function, uint16_t startAddr, uint16_t quantity, uint16_t* values);
    bool writeRegister(uint16_t addr, uint16_t value);
#endif

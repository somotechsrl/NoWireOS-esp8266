#ifndef __PROTO_10_MODBUS_RTU_CPP__
#define __PROTO_10_MODBUS_RTU_CPP__
//Extracted Prototyes
// ****************************
// src/10-modbus-rtu.cpp prototypes
// ****************************
    void begin(long baudrate);
    bool readCoils(uint8_t addr, uint8_t quantity);
    bool readHoldingRegisters(uint8_t addr, uint8_t quantity);
    bool writeRegister(uint8_t addr, uint16_t value);
    uint16_t getValue(uint8_t index);
    bool sendFrame(uint8_t* frame, uint8_t length);
    uint16_t calculateCRC(uint8_t* data, uint8_t length);
void xxsetup();
void xxloop();
#endif

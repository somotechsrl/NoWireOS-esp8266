#ifndef __PROTO_10_I2C_CPP__
#define __PROTO_10_I2C_CPP__
//Extracted Prototyes
// ****************************
// src/10-I2C.cpp prototypes
// ****************************
    void parseConfig(const String& config);
    void begin();
    uint8_t write(const uint8_t* data, uint8_t length);
    uint8_t read(uint8_t* buffer, uint8_t length);
#endif

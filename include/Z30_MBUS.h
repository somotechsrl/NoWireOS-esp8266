#ifndef __PROTO_Z30_MBUS_INO__
#define __PROTO_Z30_MBUS_INO__
//Extracted Prototyes
// ****************************
// src/Z30_MBUS.ino prototypes
// ****************************
void sendModbus();
void addModbusCall(const char *params);
void addModbusCall(const char *tag, const char *ad, uint8_t fn, uint16_t rs, uint8_t rn);
void getModbusConfig();
void setModbusSerial(uint32_t sspeed, const char *smode);
void addModbusCall(const char *tag, uint8_t ad, uint8_t fn, uint16_t rs, uint8_t rn);
void getModbusConfig(void);
void sendModbus();
#endif

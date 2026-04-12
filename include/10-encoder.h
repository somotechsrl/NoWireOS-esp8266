#ifndef __PROTO_10_ENCODER_CPP__
#define __PROTO_10_ENCODER_CPP__
//Extracted Prototyes
// ****************************
// src/10-encoder.cpp prototypes
// ****************************
void jsonInit();
const char *jsonGetBuffer();
uint16_t jsonGetBufferSize();
const char *jsonGetBase64();
void jsonClose();
void jsonCloseAll();
void jsonAddValue(int8_t value);
void jsonAddValue(int16_t value);
void jsonAddValue(int32_t value);
void jsonAddValue(char value);
void jsonAddValue(uint8_t value);
void jsonAddValue(uint16_t value);
void jsonAddValue(uint32_t value);
void jsonAddValue(float value);
void jsonAddValue(double value);
void jsonAddValue(const char *format, ...);
void jsonAddArray(const char *oname);
void jsonAddObject(const char *oname);
void jsonAddObject(const char *oname, const char *format, ...);
void jsonAddObject(const char *oname, uint8_t value);
void jsonAddObject(const char *oname, bool value);
void jsonAddObject(const char *oname, uint16_t value);
void jsonAddObject(const char *oname, uint32_t value);
void jsonAddObject(const char *oname, float value);
void jsonAddObject(const char *oname, double value);
#endif

#ifndef __PROTO_Z05_ENCODE_INO__
#define __PROTO_Z05_ENCODE_INO__
//Extracted Prototyes
// ****************************
// src/Z05_ENCODE.ino prototypes
// ****************************
void jsonClear();
void jsonInit();
const char *jsonComma();
const char *jsonGetBuffer();
uint16_t jsonGetBufferSize();
const uint8_t *jsonGetCompressedBuffer();
const uint8_t *jsonGetEncryptedBuffer();
const uint16_t jsonGetCompressedSize();
void jsonClose();
void jsonCloseAll();
void jsonAddObject(const char *oname, const char *value);
void jsonAddObject(const char *oname, const String &value);
void jsonAddObject(const char *oname, uint8_t value);
void jsonAddObject(const char *oname, bool value);
void jsonAddObject(const char *oname, uint16_t value);
void jsonAddObject(const char *oname, uint32_t value);
void jsonAddObject(const char *oname, float value);
void jsonAddObject(const char *oname, double value);
#endif

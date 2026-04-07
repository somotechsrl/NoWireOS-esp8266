#ifndef __PROTO_Z05_CONFIG_INO__
#define __PROTO_Z05_CONFIG_INO__
//Extracted Prototyes
// ****************************
// src/Z05_Config.ino prototypes
// ****************************
uint16_t configGetBit(const char *bitname);
void configEnableBit(const char *bitname);
void configDisableBit(const char *bitname);
void configGetBitsStatus();
void configGetBitsList();
void configSetBitsStatus(uint32_t bitmask);
#endif

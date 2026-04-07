#ifndef __PROTO_Z05_UTILS_INO__
#define __PROTO_Z05_UTILS_INO__
//Extracted Prototyes
// ****************************
// src/Z05_Utils.ino prototypes
// ****************************
int printf(const char *format, ...);
void jsonError(const char *id,const char *msg);
void jsonError(const char *msg);
void jsonErrorNotImplemented();
void jsonErrorNoData();
void jsonErrorNoConfig();
void jsonErrorConfigLimits();
void jsonErrorConfigAlreadySet();
void enableReboot();
void cancelReboot();
void setDebugMode(uint8_t dMode);
void debug(const char *t, const char *d);
void debug(String t, String d);
void debug(String d);
void initPixel();
void ledEnable();
void ledDisable();
void showPixel(int r, int g, int b);
void blinkPixel(int r, int g, int b);
void alivePixel();
void datalogPixel();
void commOutPixel();
void commInPixel();
void wifiConfigPixel();
void errorPixel();
void identifyPixel();
#endif

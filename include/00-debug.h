#ifndef __PROTO_00_DEBUG_CPP__
#define __PROTO_00_DEBUG_CPP__
//Extracted Prototyes
// ****************************
// src/00-debug.cpp prototypes
// ****************************
void logger_mqtt();
void logger_serial();
void logger_default();
void logger_off();
void fESP_LOGI(const char *tag,const char* format, ...);
void fESP_LOGW(const char *tag,const char* format, ...);
void fESP_LOGE(const char *tag,const char* format, ...);
void logger_mqtt();
void logger_serial();
void logger_default();
void logger_off();
#endif

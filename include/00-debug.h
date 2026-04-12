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
static void log_serial(const char *type, const char *tag,const char *fmt, va_list args);
void ESP_LOGI(const char *tag,const char* format, ...);
void ESP_LOGW(const char *tag,const char* format, ...);
void ESP_LOGE(const char *tag,const char* format, ...);
#endif

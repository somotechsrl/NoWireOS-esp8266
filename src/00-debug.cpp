#include <stdio.h>
#include "main.h"   

// default output formatter to serial -- can be extended to send logs to MQTT or other remote logging service
static void log_serial(const char *type, const char *tag,const char *fmt, va_list args) {

    char logmessage[BUFTINY],logfull[BUFTINY];
    vsnprintf(logmessage, sizeof(logmessage), fmt, args);
    snprintf(logfull,BUFTINY,"%s: %s: %s", type, tag, logmessage);

    Serial.printf("%s\n", logfull); // Print to serial for local debugging
}

// Custom debug function to send logs to MQTT
void ESP_LOGI(const char *tag,const char* format, ...) {

    char logMessage[BUFTINY];
    va_list args;
    va_start(args, format);
    log_serial("I: ", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   

// Custom debug function to send logs to MQTT
void ESP_LOGW(const char *tag,const char* format, ...) {

    char logMessage[BUFTINY];
    va_list args;
    va_start(args, format);
    log_serial("W: ", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   
    
// Custom debug function to send logs to MQTT
void ESP_LOGE(const char *tag,const char* format, ...) {

    char logMessage[BUFTINY];
    va_list args;
    va_start(args, format);
    log_serial("E: ", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   
    


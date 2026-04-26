#include <stdio.h>
#include "00-debug.h"
#include "main.h"   

#define TAG "DEBUG"

#ifndef ESP32

void logger_mqtt() {
    // Set up MQTT logging, can be extended to send logs to MQTT or other remote logging service
    // For simplicity, we will just print logs to serial in this example
    ESP_LOGI(TAG, "MQTT Logger Enabled");
}   
void logger_serial() {
    // Set up Serial logging, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Serial Logger Enabled");
}
void logger_default() {
    // Default logger, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Default Logger Enabled");
}   
void logger_off() {
    // Disable logging, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Logging Disabled"); 
}

// default output formatter to serial -- can be extended to send logs to MQTT or other remote logging service
static void log_serial(const char *type, const char *tag,const char *fmt, va_list args) {

    static char logmessage[BUFTINY],logfull[BUFTINY*2];
    vsnprintf(logmessage, sizeof(logmessage), fmt, args);
    snprintf(logfull,sizeof(logfull),"%12.3f %s %-8s: %s", millis()/1000.0, type, tag, logmessage);

    Serial.println(logfull); // Print to serial for local debugging
}
// LOgger fnction pointer for ESP_LOGx macros, can be set to different functions for different logging outputs (e.g., MQTT, Serial, etc.) as needed for more flexible logging behavior
static void (*logger)(const char *type, const char *tag,const char *fmt, va_list args) = log_serial;

// Custom debug function to send logs to MQTT
void ESP_LOGI(const char *tag,const char* format, ...) {

    va_list args;
    va_start(args, format);
    logger("I", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   

// Custom debug function to send logs to MQTT
void ESP_LOGW(const char *tag,const char* format, ...) {

    va_list args;
    va_start(args, format);
    logger("W", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   
    
// Custom debug function to send logs to MQTT
void ESP_LOGE(const char *tag,const char* format, ...) {

    va_list args;
    va_start(args, format);
    logger("E", tag, format, args); // Log to serial for local debugging
    va_end(args);
    }   
    
#else

void logger_mqtt() {
    // Set up MQTT logging, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "MQTT Logger Enabled");
}
void logger_serial() {
    // Set up Serial logging, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Serial Logger Enabled");
}
void logger_default() {
    // Default logger, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Default Logger Enabled");
    //esp_log_set_vsprintf(log_serial); // Set custom log function to send logs to MQTT
    }
void logger_off() {
    // Disable logging, can be extended to send logs to MQTT or other remote logging service
    ESP_LOGI(TAG, "Logging Disabled");
    }

#endif

include <stdio.h>
#include "main.h"   

void log_function(const char *type, const char *tag,const char *fmt, ...) {

    char logMessage[BUFTINY];
    va_list args;
    va_start(args, fmt);
    vsnprintf(logMessage, sizeof(logMessage), fmt, args);
    va_end(args);

    Serial.printf("%s\n", logMessage); // Print to serial for local debugging
}


// Custom debug function to send logs to MQTT
void ESP_LOGI(char *tag,const char* format, ...) {

    char logMessage[BUFTINY];
    va_list args;
    va_start(args, format);
    log_function(format, args); // Log to serial for local debugging
    vsnprintf(logMessage, sizeof(logMessage), format, args);
    va_end(args);

    Serial.printf("%s: %s\n", tag, logMessage); // Print to serial for local debugging
    


#include "main.h"

void ledBlink()) {
    // Led blinking to indicate activity, can be adjusted or removed as needed
    digitalWrite(LED_BUILTIN, HIGH);
    delay(BLINKDELAY);
    digitalWrite(LED_BUILTIN, LOW);
    }

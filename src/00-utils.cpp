#include "main.h"

void ledBlink() {
    // Led blinking to indicate activity, can be adjusted or removed as needed
    digitalWrite(ONBOARD_LED, HIGH);
    delay(BLINKDELAY);
    digitalWrite(ONBOARD_LED, LOW);
    }

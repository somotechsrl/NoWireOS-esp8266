#include "main.h"

#define BLINK_DELAY 1000

void ledBlink() {
    static unsigned long lastBlink = 0;
    unsigned long now = millis();
    // Led blinking to indicate activity, can be adjusted or removed as needed
    if (now - lastBlink >= BLINK_DELAY) {
        lastBlink = now;
        digitalWrite(ONBOARD_LED, LOW);
        delay(200);
        digitalWrite(ONBOARD_LED, HIGH);
    }
}


    
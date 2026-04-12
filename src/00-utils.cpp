#include "main.h"

bool led_blink_enabled = true; // global variable to control LED blinking, can be set via RPC or other means as needed for more flexible behavior
#define BLINK_DELAY 1000

void LedOn() {
    digitalWrite(ONBOARD_LED, LOW);
    }
void LedOff() {
    digitalWrite(ONBOARD_LED, HIGH);
    }
void ledBlink() {
    if(!led_blink_enabled) return;
    static unsigned long lastBlink = 0;
    unsigned long now = millis();
    // Led blinking to indicate activity, can be adjusted or removed as needed
    if (now - lastBlink >= BLINK_DELAY) {
        lastBlink = now;
        LedOn();
        delay(200);
        LedOff();
    }
}


    
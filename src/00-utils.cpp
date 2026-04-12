#include "main.h"
#include "00-utils.h"
#include <Adafruit_NeoPixel.h>

// Which pin on is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#ifdef ESP32
// Cannot use 6 as output for ESP. Pins 6-11 are connected to SPI flash. Use 16 instead.
#define LED_PIN    16
#else
#define LED_PIN    D4
#endif

// Neopixel control functions, can be extended to include more complex patterns or effects as needed for more advanced visual feedback
static bool pixelOk=false;
static Adafruit_NeoPixel leds(1, LED_PIN, NEO_GRB + NEO_KHZ800);
static void pixelInit() {
    if(pixelOk) return;
    leds.begin();
    leds.clear();
    leds.show(); // Initialize all pixels to 'off'
    pixelOk=true;
    }

static void pixelShow(int r, int g, int b) {
    pixelInit();
    leds.setPixelColor(0, leds.Color(r, g, b));
    leds.show();
    }


#define BLINK_DELAY 1000
bool led_blink_enabled = true; // global variable to control LED blinking, can be set via RPC or other means as needed for more flexible behavior

// ************************************************
// LED control functions, can be extended to include more complex patterns or effects as needed for more advanced visual feedback
// ************************************************
void ledToggle() {
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
}
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
        pixelShow(255, 0, 0); // Red blink for activity, can be adjusted to indicate different statuses or events as needed
        delay(200);
        LedOff();
        pixelShow(0, 0, 0); // Turn off pixel after blink
    }
}




    
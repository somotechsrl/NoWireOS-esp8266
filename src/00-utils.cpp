#include "main.h"
#include "00-utils.h"

#define TAG "UTIL"
#define BLINK_DELAY 1000
bool led_blink_enabled = true; // global variable to control LED blinking, can be set via RPC or other means as needed for more flexible behavior


#ifdef NEOPIXEL_PIN

// Neopixel control functions, can be extended to include more complex patterns or effects as needed for more advanced visual feedback
static bool pixelOk=false;
static Adafruit_NeoPixel leds(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static void pixelInit() {
    if(pixelOk) return;
    leds.begin();
    leds.clear();
    leds.show(); // Initialize all pixels to 'off'
    pixelOk=true;
    }

static void pixelShow(int r, int g, int b) {
    pixelInit();
    leds.clear();
    leds.setPixelColor(0, leds.Color(r, g, b));
    leds.show();
    }

void pixelBlink(int r, int g, int b) {
    if(!led_blink_enabled) return;
    static uint64_t lastBlink = millis();
    // Led blinking to indicate activity, can be adjusted or removed as needed
    if (millis() - lastBlink >= BLINK_DELAY) {
        lastBlink = millis();
        // turns off
        pixelShow(r, g, b);
        delay(200);
        // turns off  
        pixelShow(0, 0, 0); // Turn off pixel after blink
        }
    }

#else

static void pixelShow(int r, int g, int b) {}
void pixelBlink(int r, int g, int b) {}

#endif

#ifdef ONBOARD_LED

// inversion of LED state for active LOW
// can be adjusted as needed for different hardware configurations
#if defined(LED_BUILTIN) && (LED_BUILTIN==ONBOARD_LED)
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif


// ************************************************
// LED control functions, can be extended to include more complex patterns or effects as needed for more advanced visual feedback
// ************************************************
static bool ledInitialized=false;
void ledInit() {
    if(ledInitialized) return;
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, LED_OFF); // Turn off LED (active LOW)
    ledInitialized=true;
    }   

void ledToggle() {
    ledInit();
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
}
void LedOn() {
    ledInit();
    digitalWrite(ONBOARD_LED, LED_ON);
    }
void LedOff() {
    ledInit();
    digitalWrite(ONBOARD_LED, LED_OFF);
    }
void ledBlink() {
    if(!led_blink_enabled) return;
    static uint64_t lastBlink = millis();
    // Led blinking to indicate activity, can be adjusted or removed as needed
    if (millis() - lastBlink >= BLINK_DELAY) {
        // turns off
        LedOn();
        pixelShow(0, 0, 10);
        delay(50);
        lastBlink = millis();
        }

    // turns off  
    LedOff();
    pixelShow(0, 0, 0); // Turn off pixel after blink
    }

#else

void ledInit() {}   
void ledToggle() {}
void LedOn() {}
void LedOff() {}
void ledBlink() {}

#endif
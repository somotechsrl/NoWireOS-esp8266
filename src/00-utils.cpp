#include "main.h"
#include "00-utils.h"

#define TAG "UTIL"
#define BLINK_DELAY 1000
bool led_blink_enabled = true; // global variable to control LED blinking, can be set via RPC or other means as needed for more flexible behavior


#if defined(NEOPIXEL_PIN)

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

static void pixelInit() {
    // No neopixel available, nothing to initialize
    }
static void pixelShow(int r, int g, int b) {
    // No neopixel available, nothing to show
    }   

void pixelBlink(int r, int g, int b) {
    // No neopixel available, fallback to LED blink
    ledBlink();
    } 

#endif

// ************************************************
// LED control functions, can be extended to include more complex patterns or effects as needed for more advanced visual feedback
// ************************************************
static bool ledInitialized=false;
void ledInit() {
    if(ledInitialized) return;
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, HIGH); // Turn off LED (active LOW)
    ledInitialized=true;
    }   

void ledToggle() {
    ledInit();
    digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
}
void LedOn() {
    ledInit();
    digitalWrite(ONBOARD_LED, LOW);
    }
void LedOff() {
    ledInit();
    digitalWrite(ONBOARD_LED, HIGH);
    }
void ledBlink() {
    if(!led_blink_enabled) return;
    static uint64_t lastBlink = millis();
    // Led blinking to indicate activity, can be adjusted or removed as needed
    if (millis() - lastBlink >= BLINK_DELAY) {
        lastBlink = millis();
        // turns off
        LedOn();
        pixelShow(0, 0, 10);
        delay(200);
        // turns off  
        LedOff();
        pixelShow(0, 0, 0); // Turn off pixel after blink
    }
}


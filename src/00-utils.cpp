#include "main.h"
#include "00-utils.h"
#include <Adafruit_NeoPixel.h>

#define BLINK_DELAY 1000
bool led_blink_enabled = true; // global variable to control LED blinking, can be set via RPC or other means as needed for more flexible behavior

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

// updates time and return time value
#define NTP_CHECK_TIME 8*3600*2 // 2 days in seconds, used to check if time is set before 1970, can be adjusted as needed for more accurate checks in real-world applications
bool ntpSet() {
    ESP_LOGI(TAG, "Updating NTP time...");      
    time_t now = time(nullptr);
    if (now < NTP_CHECK_TIME) { // if time is before 1970, update time
        configTime(0, 0, NTP_SERVER);
        time_t now = time(nullptr);
        while (now < NTP_CHECK_TIME) {
            delay(500);
            now = time(nullptr);
            ESP_LOGI(TAG, "Waiting for NTP time sync...");
        }

    time_t now = time(nullptr);
    return now >= NTP_CHECK_TIME; // checks if time is set before 1970, can be adjusted as needed for more accurate checks in real-world applications
    }

    
    
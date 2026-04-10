#include "main.h"
#include "10-wifi-provision.h"

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(100);
    
    EEPROM.begin(512);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    EEPROM.get(0, wifiConfig);
    
    if (strlen(wifiConfig.ssid) > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiConfig.ssid, wifiConfig.password);
        Serial.println("Connecting to WiFi...");
    } else {
        startProvisioningMode();
    }

    Serial.println("\n\nSystem started!");
    
    // Initialize pins, sensors, etc.
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

    void loop() {
    checkResetButton();
    
    if (provisionMode) {
        server.handleClient();
    } else if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        // Main application logic
    }
    
    delay(1000);
    // Main program logic
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
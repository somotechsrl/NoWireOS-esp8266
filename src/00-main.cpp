#include "main.h"
#include "10-modbus-tcp.h"
#include "10-wifi-provision.h"

// 10 msecs time step, can be adjusted as needed for more responsive behavior or lower power consumption
#define TIME_INCREMENT 10

// common mac address and uuid for device, can be used for identification in MQTT topics, etc.
char mac[18], uuid[33];

// timestep for modbus polling, can be adjusted as needed for more frequent updates or lower network traffic
uint32_t timestep=10000; // 10 secs

// TAG for logging
#define TAG "MAIN"

void setup() {

    // Initialize serial communication
    Serial.begin(115200);
    delay(100);
   
    ESP_LOGI(TAG, "Booting up...");

    EEPROM.begin(512);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    EEPROM.get(0, wifiConfig);
    
    if (strlen(wifiConfig.ssid) > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiConfig.ssid, wifiConfig.password);
        ESP_LOGI(TAG, "Connecting to WiFi...");
    } else {
        startProvisioningMode();
    }

    ESP_LOGI(TAG, "System started!");

    // Initialize pins, sensors, etc.
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

    // time stepper
    static uint32_t currenttime=0;
    
    if (provisionMode) {
        server.handleClient();
    } else if (WiFi.status() == WL_CONNECTED && currenttime >= timestep) {
        readModbusTcp();
        currenttime = 0; // Reset timer after reading Modbus
    } else if(WiFi.status() == WL_CONNECTED) {
       checkResetButton();
    } else if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "WiFi not connected. Attempting to reconnect...");
        WiFi.reconnect();
        delay(5000); // Wait before retrying
        }

    // increents tiestep
    currenttime += TIME_INCREMENT;
    
    ledBlink();
}
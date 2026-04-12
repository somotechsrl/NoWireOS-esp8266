#include "main.h"
#include "10-modbus-tcp.h"
#include "10-wifi-provision.h"
#include "20-mqtt.h"

// 10 msecs time step, can be adjusted as needed for more responsive behavior or lower power consumption
#define TIME_INCREMENT 10


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
    
    netInit(); // Initialize network, can be extended to include Ethernet or other interfaces as needed for more flexible connectivity options

    if (strlen(wifiConfig.ssid) > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiConfig.ssid, wifiConfig.password);
        ESP_LOGI(TAG, "Connecting to WiFi...");
    } else {
        startProvisioningMode();
    }

    //mqttInit();
    ESP_LOGI(TAG, "System started!");

    // Initialize MQTT client, connection is handled in loop()
    mqttInit();
}

void loop() {

    // time stepper
    static uint64_t cmillis=millis();

 
    if(millis()<cmillis) {
        cmillis=millis(); // reset timer if overflow
        }
    
    if (provisionMode) {
        server.handleClient();
        return;
        }

    if (WiFi.status() == WL_CONNECTED) {
        
        mqttPoll(); // Handle MQTT communication, will attempt reconnect if connection is lost

        if(millis()-cmillis > timestep) {
            readModbusTcp();
            cmillis = millis(); // Reset timer after reading Modbus
            ESP_LOGI(TAG, "Modbus data read successfully, waiting %lu", timestep);
            }

        checkResetButton();
        // increents tiestep
        }

    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "WiFi not connected. Attempting to reconnect...");
        WiFi.reconnect();
        delay(5000); // Wait before retrying
        }

    //ESP_LOGI(TAG, "Current timestep: %llu, %llu", timestep, cmillis);
    
    ledBlink();
}
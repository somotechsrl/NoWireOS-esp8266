#include "main.h"
#include "20-modbus-master.h"
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

    if(wifiCheck()) {
               
        delay(100);
        // mqtt active and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
        if(mqttPoll() && millis()-cmillis > timestep) {
            // calls modbus master yask, rads congif and esxecute
            modbusMasterTask();
            cmillis = millis(); // Reset timer after reading Modbus
            ESP_LOGI(TAG, "Modbus data read successfully, waiting %lu", timestep);
            }

        }

    ledBlink();
}
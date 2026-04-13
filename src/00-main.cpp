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
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    
    // sets net params
    netInit(); 
    
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
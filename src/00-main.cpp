#include "main.h"
#include "20-modbus-master.h"
#include "10-GPIO.h"
#include "10-wifi-provision.h"
#include "20-mqtt.h"

// 10 msecs time step, can be adjusted as needed for more responsive behavior or lower power consumption
#define TIME_INCREMENT 10


// TAG for logging
#define TAG "MAIN"

void setup() {
   
    //init serial speed
    Serial.begin(115200);
    logger_serial(); 

    ESP_LOGI(TAG, "Booting up...");

    netInit(); 
    mqttInit();
}

void loop() {

    // time stepper
    static uint64_t cmillis=millis();

 
    if(millis()<cmillis) {
        cmillis=millis(); // reset timer if overflow
        }

    //ESP_LOGI(TAG, "Checking WiFi and MQTT status...");
    // wifi connected, mqtt active, and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
    if(wifiCheck()) {
        
        // checks mqtt status
        mqttPoll();
        delay(100);

        // mqtt active and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
        if(millis()-cmillis > timestep) {
            // calls modbus master task, reads config and executes
            // TO DO ... different timings for each task
            modbusMasterTask();
            gpioMasterTask();
            sysInfoMasterTask();
            cmillis = millis(); // Reset timer after reading Modbus
            }

        }

    ledBlink();
}
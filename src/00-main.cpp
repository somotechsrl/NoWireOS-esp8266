#include "main.h"
#include "20-modbus-master.h"
#include "10-GPIO.h"
#include "10-wifi-provision.h"
#include "20-mqtt.h"
#include "20-rpc-utils.h"

// 10 msecs time step, can be adjusted as needed for more responsive behavior or lower power consumption
#define TIME_INCREMENT 10
#define NTP_UPDATE_INTERVAL 3600000 // 1 hour in milliseconds, can be adjusted as needed for more frequent updates or lower network traffic in real-world applications

// TAG for logging
#define TAG "MAIN"

void setup() {
   

    //init serial speed
    Serial.begin(SERIAL_SPEED);
    logger_serial(); 
    Serial.println();
    Serial.println("NoWireOS Starting...");

    ESP_LOGI(TAG, "Booting up...");

    netInit(); 
    mqttInit();
}

void loop() {

    // time stepper
    static uint64_t mbumillis=millis();
    static uint64_t sysmillis=millis();
 
    if(millis()<mbumillis) {
        mbumillis=millis(); // reset timer if overflow
        }

    //ESP_LOGI(TAG, "Checking WiFi and MQTT status...");
    // wifi connected, mqtt active, and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
    if(netCheck()) {
        
        // checks mqtt status
        mqttPoll();
        delay(100);

         // mqtt active and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
        if(millis()-mbumillis > mbutimestep) {
            // calls modbus master task, reads config and executes
            // TO DO ... different timings for each task
            modbusMasterTask();
            }

        // default system timestep is 5min, will be changed 
        // see rpcmanager module for dynamic configuration via RPC, can be adjusted as needed for more frequent updates or lower network traffic in real-world applications        
        if(millis()-sysmillis > systimestep) {
            gpioMasterTask();
            sysGetInfoTask();
            sysmillis  = millis(); // Reset timer after reading Modbus
            }

        // mqtt active and time step reached, can be adjusted as needed for more responsive behavior or lower power consumption
        // see rpcmanager module for dynamic configuration via RPC, can be adjusted as needed for more frequent updates or lower network traffic in real-world applications
        if(millis()-mbumillis > mbutimestep) {
            // calls modbus master task, reads config and executes
            // TO DO ... different timings for each task
            modbusMasterTask();
            mbumillis = millis(); // Reset timer after reading Modbus
            }

        }

    ledBlink();
}
#include "main.h"
#include "00-debug.h"
#include "10-modbus-tcp.h"
#include "10-wifi-provision.h"

#define TIME_INCREMENT 1000
char mac[18], uuid[33];
uint32_t timestep=10000; // 10 secs
static const char* TAG = "MAIN";

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
        ESP_LOGI(TAG, "Connecting to WiFi...");
    } else {
        startProvisioningMode();
    }

    ESP_LOGI(TAG, "\n\nSystem started!");

    // Initialize pins, sensors, etc.
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

    // time stepper
    static uint32_t currenttime=0;

    checkResetButton();
    Serial.println(currenttime);
    
    if (provisionMode) {
        server.handleClient();
    } else if (WiFi.status() == WL_CONNECTED && currenttime >= timestep) {
        strcpy(mac, WiFi.macAddress().c_str());
        strcpy(uuid, WiFi.macAddress().c_str());
        readModbusTcp();
        currenttime = 0; // Reset timer after reading Modbus
    } else if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "WiFi not connected. Attempting to reconnect...");
        WiFi.reconnect();
        delay(5000); // Wait before retrying
        }

    // increents tiestep
    currenttime += TIME_INCREMENT;
    
    // Main program logic
    digitalWrite(LED_BUILTIN, HIGH);
    delay(TIME_INCREMENT-100);
    
    digitalWrite(LED_BUILTIN, LOW);
    delay(TIME_INCREMENT-900);
}
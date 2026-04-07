#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

struct ApConfig {
    char ssid[32];
    char password[64];
    uint8_t valid;
};

const int EEPROM_SIZE = 512;
const int EEPROM_ADDR = 0;
const char DEFAULT_SSID[] = "NoWireAP";
const char DEFAULT_PASSWORD[] = "nopassword";

ApConfig wifiConfig;

void loadApConfig() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(EEPROM_ADDR, wifiConfig);
    if (wifiConfig.valid != 0xA5) {
        strncpy(wifiConfig.ssid, DEFAULT_SSID, sizeof(wifiConfig.ssid));
        strncpy(wifiConfig.password, DEFAULT_PASSWORD, sizeof(wifiConfig.password));
        wifiConfig.valid = 0xA5;
        EEPROM.put(EEPROM_ADDR, wifiConfig);
        EEPROM.commit();
    }
}

void saveApConfig(const char* ssid, const char* password) {
    strncpy(wifiConfig.ssid, ssid, sizeof(wifiConfig.ssid) - 1);
    wifiConfig.ssid[sizeof(wifiConfig.ssid) - 1] = '\0';
    strncpy(wifiConfig.password, password, sizeof(wifiConfig.password) - 1);
    wifiConfig.password[sizeof(wifiConfig.password) - 1] = '\0';
    wifiConfig.valid = 0xA5;

    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_ADDR, wifiConfig);
    EEPROM.commit();
}

void setupWifiAp() {
    loadApConfig();

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP);

    if (strlen(wifiConfig.password) >= 8) {
        WiFi.softAP(wifiConfig.ssid, wifiConfig.password);
    } else {
        WiFi.softAP(wifiConfig.ssid);
    }

    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("AP started: %s / %s\n", wifiConfig.ssid, wifiConfig.password);
    Serial.printf("AP IP: %s\n", apIP.toString().c_str());
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // Example: Save a new AP configuration
    saveApConfig("NoWireOS", "12345678");

    setupWifiAp();
}

void loop() {
    // Put your main code here, to run repeatedly
}
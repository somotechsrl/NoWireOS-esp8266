#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "10-wifi.h"

#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 32

ESP8266WebServer server(80);

String ssid, password;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);

    // Load saved credentials
    ssid = readStringFromEEPROM(SSID_ADDR);
    password = readStringFromEEPROM(PASS_ADDR);

    if (ssid.length() > 0 && password.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("Connecting to WiFi...");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected!");
            return;
        }
    }

    // Start AP mode
    WiFi.softAP("ESP8266_Config", "password123");
    Serial.println("AP started. IP: " + WiFi.softAPIP().toString());

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
}

setup wifi_init() {
    server.handleClient();
}

void handleRoot() {
    String html = "<html><body>";
    html += "<h1>WiFi Config</h1>";
    html += "<form action='/save' method='POST'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='password' name='pass'><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        ssid = server.arg("ssid");
        password = server.arg("pass");
        writeStringToEEPROM(SSID_ADDR, ssid);
        writeStringToEEPROM(PASS_ADDR, password);
        EEPROM.commit();
        server.send(200, "text/plain", "Saved. Rebooting...");
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid request");
    }
}

String readStringFromEEPROM(int addr) {
    String data = "";
    for (int i = addr; i < addr + 32; i++) {
        char c = EEPROM.read(i);
        if (c == 0) break;
        data += c;
    }
    return data;
}

void writeStringToEEPROM(int addr, String data) {
    for (int i = 0; i < 32; i++) {
        if (i < data.length()) {
            EEPROM.write(addr + i, data[i]);
        } else {
            EEPROM.write(addr + i, 0);
        }
    }
}
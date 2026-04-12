#include "main.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define TAG "WIFI_PROV"

ESP8266WebServer server(80);
bool provisionMode = false;
WiFiConfig wifiConfig;

// uuid and mac
String uuid, mac;
void netInit() {

  WiFi.begin();
  uuid = mac = WiFi.macAddress();
  ESP_LOGI(TAG, "MAC: %s", mac.c_str());
  uuid.replace(":", "");
  ESP_LOGI(TAG, "UUID: %s", uuid.c_str());

}

static void handleRoot() {
    String html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>WiFi Provisioning</title>
            <style>
                body { font-family: Arial; margin: 20px; }
                input { padding: 8px; margin: 5px; width: 200px; }
                button { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }
            </style>
        </head>
        <body>
            <h1>WiFi Provision</h1>
            <form method="POST" action="/provision">
                <label>SSID:</label><br>
                <input type="text" name="ssid" required><br>
                <label>Password:</label><br>
                <input type="password" name="password"><br>
                <button type="submit">Connect</button>
            </form>
        </body>
        </html>
    )";
    server.send(200, "text/html", html);
}

static void handleProvision() {
    if (server.method() == HTTP_POST) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        
        ssid.toCharArray(wifiConfig.ssid, 32);
        password.toCharArray(wifiConfig.password, 64);
        
        EEPROM.put(0, wifiConfig);
        EEPROM.commit();
        
        server.send(200, "text/html", "<h1>Credentials saved! Restarting...</h1>");
        delay(1000);
        ESP.restart();
    }
}

void startProvisioningMode() {
    provisionMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP("NoWireOS-Setup", "12345678");
    
    server.on("/", handleRoot);
    server.on("/provision", handleProvision);
    server.begin();
    
    Serial.println("Provisioning Mode Started");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void checkResetButton() {
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(RESET_BUTTON_PIN) == LOW) {
            delay(3000);
            if (digitalRead(RESET_BUTTON_PIN) == LOW) {
                Serial.println("Reset button pressed - entering provisioning mode");
                memset(&wifiConfig, 0, sizeof(wifiConfig));
                EEPROM.put(0, wifiConfig);
                EEPROM.commit();
                startProvisioningMode();
            }
        }
    }
}

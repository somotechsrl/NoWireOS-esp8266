#include "main.h"
#include <EEPROM.h>

#define TAG "WIFI_PROV"

#ifdef ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif
bool provisionMode = false;
WiFiConfig wifiConfig;

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

static void startProvisioningMode() {
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

static void checkResetButton() {
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

// uuid and mac
String uuid, mac;
void netInit() {

  WiFi.begin();
  uuid = mac = WiFi.macAddress();
  ESP_LOGI(TAG, "MAC: %s", mac.c_str());
  uuid.replace(":", "");
  ESP_LOGI(TAG, "UUID: %s", uuid.c_str());

}

void wifiCheck() {

    // Checks if Wifi is connected and reset button
    if(WiFi.status() == WL_CONNECTED) {
        checkResetButton();
        return;
        }
    }

    // tries to ceonnect to wifi, if fails after timeout, enters provisioning mode
    ESP_LOGI(TAG, "Connecting to WiFi '%s'...", wifiConfig.ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    //WiFi.begin("DeepBlue","!eralottoluglio");
    uint32_t startAttemptTime = millis();
    const uint32_t wifiTimeout = 20000;
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        delay(500);
        Serial.print(".");
    }

    if(WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "WiFi not connected, entering provisioning mode");
        startProvisioningMode();
    } else {
        ESP_LOGI(TAG, "WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
    }

}
#include "main.h"
#include <EEPROM.h>

#define TAG "WIFI_PROV"

// Wifi stuff platform specific
#ifdef ESP32
#include "WebServer.h"
static WebServer server(80);
#else
#include <ESP8266WebServer.h>
static ESP8266WebServer server(80);
#endif

static struct  {
    char ssid[64];
    char password[128];
} wifiConfig;


static time_t datetime;
const time_t *getTime() {
    time(&datetime);
    return &datetime;
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
        
        ssid.toCharArray(wifiConfig.ssid, 64);
        password.toCharArray(wifiConfig.password, 128);
        
        EEPROM.put(0, wifiConfig);
        EEPROM.commit();
        
        server.send(200, "text/html", "<h1>Credentials saved! Restarting...</h1>");
        delay(1000);
        ESP.restart();
    }
}

static void startProvisioningMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("NoWireOS-Setup", "12345678");
    
    server.on("/", handleRoot);
    server.on("/provision", handleProvision);
    server.begin();
    
    ESP_LOGI(TAG, "Provisioning Mode Started");
    ESP_LOGI(TAG, "AP IP: %s", WiFi.softAPIP().toString().c_str());

    ESP_LOGI(TAG, "Connect to 'NoWireOS-Setup' with password '12345678' to configure WiFi");
}

bool WiFiReset() {
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    EEPROM.put(0, wifiConfig);
    EEPROM.commit();
    ESP_LOGI(TAG, "WiFi credentials cleared, restarting...");
    ESP_LOGI(TAG, "Entering provisioning mode...");
    startProvisioningMode();
    return true;
    }

static void checkResetButton() {
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(RESET_BUTTON_PIN) == LOW) {
            delay(3000);
            if (digitalRead(RESET_BUTTON_PIN) == LOW) {
                ESP_LOGI(TAG, "Reset button pressed - entering provisioning mode");
                WiFiReset();
            }
        }
    }
}

// uuid and mac
String uuid, mac;
void netInit() {

    ESP_LOGI(TAG, "Initializing network...");

    WiFi.begin();
    uuid = mac = WiFi.macAddress();
    ESP_LOGI(TAG, "MAC: %s", mac.c_str());
    uuid.replace(":", "");
    ESP_LOGI(TAG, "UUID: %s", uuid.c_str());

    // sets wifi reset button
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    }


bool wifiCheck() {

    // Provisonig mode, not STA, handle provisioning server
    if(WiFi.getMode() == WIFI_AP) {
        server.handleClient();
        return false;
    }

    // Checks if Wifi is connected and reset button
    if(WiFi.status() == WL_CONNECTED) {
        checkResetButton();
        return true;
        }

    // gets wifi config from EEPROM, if ssid is empty, enters provisioning mode
    if(!EEPROM.begin(512)) {
        ESP_LOGE(TAG, "Failed to initialize EEPROM");
        startProvisioningMode();
        server.handleClient();
        return false;
    }

    // reads config from EEPROM, if ssid is empty, enters provisioning mode
    memset(&wifiConfig,0,sizeof(wifiConfig));
    EEPROM.get(0, wifiConfig);
    if(strlen(wifiConfig.ssid) == 0 || strlen(wifiConfig.ssid) > 31) {
        ESP_LOGW(TAG, "No WiFi or wrong credentials found, entering provisioning mode");
        startProvisioningMode();
        return false;
    }

    // tries to connect to wifi, if fails after timeout, enters provisioning mode
    ESP_LOGI(TAG, "Connecting to WiFi '%s'...", wifiConfig.ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    //WiFi.begin("DeepBlue","!eralottoluglio");

    Serial.println();
    uint32_t startAttemptTime = millis();
    const uint32_t wifiTimeout = 20000;
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if(WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
        return true;
        }

    ESP_LOGW(TAG, "WiFi not connected, entering provisioning mode");
    startProvisioningMode();
    return false;
    }


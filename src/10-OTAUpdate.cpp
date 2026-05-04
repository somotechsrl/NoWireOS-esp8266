#include "main.h"
#define TAG "OTAUpdate"

#ifdef HAS_OTA

#include <HTTPClient.h>
#include <Update.h>

void handleOTAUpdate(const char* url) {
  HTTPClient http;
  
  ESP_LOGI(TAG, "Starting OTA update from: %s", url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode != HTTP_CODE_OK) {
    ESP_LOGE(TAG, "HTTP error: %d", httpCode);    
    http.end();
    return;
  }
  
  int contentLength = http.getSize();
  if (contentLength <= 0) {
    ESP_LOGE(TAG, "Invalid content length");
    http.end();
    return;
  }
  
  if (!Update.begin(contentLength)) {
    ESP_LOGE(TAG, "Not enough space for OTA");
    http.end();
    return;
  }
  
  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);
  
  if (written != contentLength) {
    ESP_LOGE(TAG, "Written only: %d/%d", written, contentLength);
    Update.end();
    http.end();
    return;
  }
  
  if (Update.end()) {
    ESP_LOGI(TAG, "OTA update completed successfully");
    ESP.restart();
  } else {
    ESP_LOGE(TAG, "OTA update failed");
  }
  
  http.end();
}

void registerOTACommands() {
  rpc.registerCommand("ota", [](const char* args) {
    handleOTAUpdate(args);
    return "OTA update initiated";
  });
}
#else

void registerOTACommands() {
  // No OTA support on this platform
  ESP_LOGW(TAG, "OTA update not supported on this platform");
}   

void handleOTAUpdate(const char* url) {
  ESP_LOGW(TAG, "OTA update not supported on this platform");
}   

#endif
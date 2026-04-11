#include <Arduino.h>
#include "00-debug.h"
#include <ESP8266WiFi.h>

static WiFiClient client;
static uint8_t mbAddress = 1; // Modbus unit ID, can be configured as needed
static uint16_t transactionId;

static void sendRequest(uint8_t* request, uint16_t length) {
    client.write(request, length);
   }

static bool receiveResponse(uint8_t* response, uint16_t maxLength, uint16_t& length) {
    length = 0;
    unsigned long timeout = millis() + 2000;
        
    while (millis() < timeout && length < maxLength) {
        if (client.available()) {
            response[length++] = client.read();
            }
        }
    return length > 0;
    }

bool modbusTcpConnect(const char *host, int port, uint8_t unitId) {
    mbAddress = unitId;
    ESP_LOGI("MODBUS", "Connecting to Modbus TCP server at %s:%d with unit ID %d", host, port, unitId);
    return client.connect(host, port);
    }

void modbusTcpDisconnect() {
    ESP_LOGI("MODBUS", "Disconnecting from Modbus TCP server");
    client.stop();
    }

uint16_t modbusTcpReadRegisters(uint8_t function, uint16_t startAddr, uint16_t &quantity) {

    // buffer for register values, can be expanded as needed for larger reads, currently set to max of 125 registers for Modbus TCP
    static uint8_t dest[MODBUS_TCP_MAX_REGS];
    
    uint8_t request[12];
    transactionId++;

    request[0] = (transactionId >> 8) & 0xFF;
    request[1] = transactionId & 0xFF;
    request[2] = 0x00;  // Protocol ID high
    request[3] = 0x00;  // Protocol ID low
    request[4] = 0x00;  // Length high
    request[5] = 0x06;  // Length low
    request[6] = mbAddress;  // Unit ID
    request[7] = function;  // Function code
    request[8] = (startAddr >> 8) & 0xFF;
    request[9] = startAddr & 0xFF;
    request[10] = (quantity >> 8) & 0xFF;
    request[11] = quantity & 0xFF;

    sendRequest(request, 12);

    // read response, check for errors, extract register values into provided buffer, and add to json response for mqtt transmission, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    static uint8_t response[MODBUS_MAX_TCP_RESPONSE];
    uint16_t respLength;
    if (!receiveResponse(dest, sizeof(response), respLength)) {
        return NULL;
        }

    // check function field for errors
    if (response[7] & 0x80) {
        ESP_LOGE("MODBUS", "Modbus error response received: function code 0x%02X, error code 0x%02X", response[7], response[8]);
        return NULL;
        }

    if (response[7] != function) {
        ESP_LOGE("MODBUS", "Unexpected function code in response: 0x%02X", response[7]);
        return NULL;
    }

    // return pointer to register values in response, can be used for further processing if needed, currently we are just adding values to json response for mqtt transmission
    return (uint16_t*)(response+9);
    }

static bool writeRegister(uint16_t addr, uint16_t value) {
    uint8_t request[12];
    transactionId++;

    request[0] = (transactionId >> 8) & 0xFF;
    request[1] = transactionId & 0xFF;
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x06;
    request[6] = mbAddress;  // Unit ID
    request[7] = 0x10;  // Function code: Write Multiple Registers
    request[8] = (addr >> 8) & 0xFF;
    request[9] = addr & 0xFF;
    request[10] = 0x00;
    request[11] = 0x01;

    sendRequest(request, 12);

    uint8_t response[256];
    uint16_t respLength;
    return receiveResponse(response, 256, respLength) && response[7] == 0x10;
    }

void readModbusTcp() {
    if (modbusTcpConnect("192.168.43.169", 502, 1)) {
        
        uint16_t *values;
    
        if ((values=modbusTcpReadRegisters(0x03, 4096, 10)) != NULL) {
            for (int i = 0; i < 10; i++) {
                Serial.println(values[i]);
            }
        }

        modbusTcpDisconnect();
    }
}   
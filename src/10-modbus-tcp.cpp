#include <Arduino.h>
#include <ESP8266WiFi.h>

static WiFiClient client;
static const char* host;
static uint16_t port;
static uint8_t unitId;
static uint16_t transactionId;

static void sendRequest(uint8_t* request, uint16_t length) {
    client.write(request, length);
   }

bool receiveResponse(uint8_t* response, uint16_t maxLength, uint16_t& length) {
    length = 0;
    unsigned long timeout = millis() + 2000;
        
    while (millis() < timeout && length < maxLength) {
        if (client.available()) {
            response[length++] = client.read();
            }
        }
    return length > 0;
    }

static bool modbusTcpConnect(const char *host, int port  = 502   , uint8_t unitId = 1) {
    return client.connect(host, port);
    }

static void modbusTcpDisconnect() {
    client.stop();
    }

static bool modbusTcpReadRegisters(uint8_t function, uint16_t startAddr, uint16_t quantity, uint16_t* values) {
    uint8_t request[12];
    transactionId++;

    request[0] = (transactionId >> 8) & 0xFF;
    request[1] = transactionId & 0xFF;
    request[2] = 0x00;  // Protocol ID high
    request[3] = 0x00;  // Protocol ID low
    request[4] = 0x00;  // Length high
    request[5] = 0x06;  // Length low
    request[6] = unitId;
    request[7] = function;  // Function code
    request[8] = (startAddr >> 8) & 0xFF;
    request[9] = startAddr & 0xFF;
    request[10] = (quantity >> 8) & 0xFF;
    request[11] = quantity & 0xFF;

    sendRequest(request, 12);

    uint8_t response[256];
    uint16_t respLength;
    if (!receiveResponse(response, 256, respLength)) {
        return false;
        }

    if (response[7] != 0x03) {
        return false;
    }

    uint8_t byteCount = response[8];
    for (int i = 0; i < quantity && i * 2 < byteCount; i++) {
        values[i] = (response[9 + i * 2] << 8) | response[10 + i * 2];
        }

    return true;
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
    request[6] = unitId;
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
        uint16_t values[10];
        if (modbusTcpReadRegisters(0x03, 4096, 10, values)) {
            for (int i = 0; i < 10; i++) {
                Serial.println(values[i]);
            }
        }
        modbusTcpDisconnect();
    }
}   
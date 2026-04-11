#include <Arduino.h>
#include "00-debug.h"
#include <ESP8266WiFi.h>
#nclude "10-encoder.h"

static WiFiClient client;
static uint16_t transactionId;

#define TAG "MBTCP"
#define MODBUS_TCP_MAX_REGS 125
#define MODBUS_MAX_TCP_RESPONSE (9 + 2*MODBUS_TCP_MAX_REGS)

static void sendRequest(uint8_t* request, uint16_t length) {
    client.write(request, length);
   }

static bool receiveResponse(uint8_t* response, uint16_t maxLength, uint16_t *length) {
    length = 0;
    unsigned long timeout = millis() + 2000;
        
    while (millis() < timeout && *length < maxLength) {
        if (client.available()) {
            response[(*length)++] = client.read();
            }
        }
    return *length > 0;
    }

bool modbusTcpConnect(const char *host, int port, uint8_t unitId) {
    ESP_LOGI("MODBUS", "Connecting to Modbus TCP server at %s:%d", host, port);
    return client.connect(host, port);
    }

void modbusTcpDisconnect() {
    ESP_LOGI("MODBUS", "Disconnecting from Modbus TCP server");
    client.stop();
    }

static uint8_t modbus_error;
static uint16_t *modbusTcpRead(uint8_t unitId,uint8_t function, uint16_t startAddr, uint8_t quantity) {

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
    request[6] = unitId;  // Unit ID
    request[7] = function;  // Function code
    request[8] = (startAddr >> 8) & 0xFF;
    request[9] = startAddr & 0xFF;
    request[10] = (quantity >> 8) & 0xFF;
    request[11] = quantity & 0xFF;

    sendRequest(request, 12);

    // read response, check for errors, extract register values into provided buffer, and add to json response for mqtt transmission, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    uint16_t respLength=0;
    static uint8_t response[MODBUS_MAX_TCP_RESPONSE];
    if (!receiveResponse(dest, sizeof(response), &respLength)) {
        modbus_error=0xf0; // custom error code for no response
        ESP_LOGE("MODBUS", "No response received from Modbus TCP server");
        return NULL;
        }

    uint8_t *header=&response[0]; // response starts with 7 byte header: transaction id (2), protocol id (2), length (2), unit id (1)
    uint8_t *pdu=&response[7];  // pdu starts after header, first byte is function code

    // basic validation of response header, can be expanded later to include more detailed validation and error handling as needed for robustness in real-world applications
    modbus_error=0;
    uint16_t recv_transaction = (header[0] << 8) | header[1];
    uint16_t protocol_id = (header[2] << 8) | header[3];
    uint16_t length = (header[4] << 8) | header[5];
    uint8_t recv_unit = header[6];

    if (recv_transaction != (transactionId - 1)) {
        ESP_LOGW(TAG, "transaction id mismatch: expected %u got %u", transactionId - 1, recv_transaction);
        modbus_error=0xfa; // custom error code for transaction id mismatch
        return NULL;
    }
    if (protocol_id != 0) {
        ESP_LOGW(TAG, "unexpected Modbus protocol id %u", protocol_id);
        modbus_error=0xfb; // custom error code for protocol id mismatch
        return NULL;
    }
    if (recv_unit != unitId) {
        ESP_LOGW(TAG, "unexpected unit id %u", recv_unit);
        modbus_error=0xfc; // custom error code for unit id mismatch
        return NULL;
    }
    if (length < 2) {
        ESP_LOGE(TAG, "invalid Modbus response length %u", length);
        modbus_error=0xfd; // custom error code for invalid response
        return NULL;
    }

    size_t pdu_length = length - 1;
    if (pdu_length > 253) {
        ESP_LOGE(TAG, "too large Modbus PDU length %zu", pdu_length);
        modbus_error=0xfe; // custom error code for invalid response
        return NULL;
    }

    // error response has function code with MSB set and error code in byte 8 of response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (pdu[0] & 0x80) {
        ESP_LOGE("MODBUS", "Modbus error response received: function code 0x%02X, error code 0x%02X", pdu[0], pdu[1]);
        modbus_error=pdu[1]; // error code is in byte 8 of response
        return NULL;
        }
    
    //unexpected function code in response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (pdu[0] != function) {
        ESP_LOGE("MODBUS", "Unexpected function code in response: 0x%02X", pdu[0]);
        modbus_error=pdu[0]; // error code is in byte 8 of response
        return NULL;
    }

    if(pdu[1]!=quantity*2) {
        ESP_LOGE("MODBUS", "Unexpected byte count in response: expected %d got %d", quantity*2, pdu[1]);
        modbus_error=0xff; // custom error code for invalid response
        return NULL;
    }


    // return pointer to register values in response starting from function code, can be used for further processing if needed, currently we are just adding values to json response for mqtt transmission
    return (uint16_t*)pdu[1];
    }

#ifdef __MODBUS_TCP_WRITE_ENABLED__ // can be enabled as needed for write functionality, currently we are just implementing read functionality for simplicity, can be expanded later to include write functionality as needed
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
#endif


// Default entry for modbus tcp client task, will be called by modbus client task loop for each call in config
uint16_t *modbusTcpReadJson(uint8_t unit_id, uint8_t func, uint16_t start_address, uint16_t quantity) {

    uint16_t *response;
    uint8_t respLength;
    char jobjectid[64];
    sprintf(jobjectid,"x%04x",start_address);

    // reads data from modbus tcp server, response is array of uint16_t, jsonAddValue will add as number, if want to add as string need to convert to string first
    response=modbusTcpRead(unit_id , func, start_address, quantity);

    jsonAddArray(jobjectid);
    jsonAddValue_uint8_t(func);
    jsonAddValue_uint16_t(modbus_error);
    jsonAddValue_uint16_t(start_address);

    // get values for non null response, if response is null, it means there was an error, modbus_error variable will have error code, if response is not null, modbus_error should be 0
    for (uint16_t i = 0; response && i < quantity; ++i) {
        jsonAddValue_uint16_t(response[i]);
        }

    jsonClose();

    return response;
}

void readModbusTcp() {
    char *host="rpc.somotech.it"
    uint16_t port=502;
    uint8_t unit_id=1;
    uint8_t func=3; // read holding registers
    uint16_t start_address=4096;
    uint16_t quantity=10;

    if (modbusTcpConnect(host, port, unit_id)) {
        ModbusTcpReadJson(unit_id, func, start_address, quantity);
        modbusTcpDisconnect();
        } else {
        ESP_LOGE(TAG, "Failed to connect to Modbus TCP server: %s:%d", host, port);
        } 

}
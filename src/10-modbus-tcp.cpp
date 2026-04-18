// Modbus TCP client implementation for ESP32 and ESP8266, can be extended later to include additional features such as write functionality, support for more function codes, or more detailed error handling as needed for robustness in real-world applications --- IGNORE ---
#include "main.h"
#include "10-encoder.h"

#define TAG "MBTCP"
#define MODBUS_TCP_BUFFER  256

#ifndef CUBE_CELL

static WiFiClient client;
static uint16_t transactionId;
static uint16_t modbus_timeout_ms=2000; 

void setModbusTimeout(uint16_t timeout_ms) {
    // sets socket timeout for Modbus TCP client, can be used to adjust responsiveness and error handling in real-world applications
    modbus_timeout_ms = timeout_ms;
    ESP_LOGI(TAG, "Modbus TCP timeout set to %u ms", modbus_timeout_ms);
    }
uint16_t getModbusTimeout() {
    // retrieves the current socket timeout for Modbus TCP client, can be used to adjust responsiveness and error handling in real-world applications
    return modbus_timeout_ms;
    }

static void sendRequest(uint8_t* request, uint16_t length) {
    client.write(request, length);
   }

static uint16_t receiveResponse(uint8_t* response, uint16_t maxLength) {

    uint16_t length=0;
    unsigned long timeout = millis() + modbus_timeout_ms;
        
    while (millis() < timeout && length < maxLength) {
        if (client.available()) {
            length += client.read(response + length, maxLength - length);
            }
        }
    ESP_LOGI(TAG,"Received %u of %u bytes from Modbus TCP server", length, maxLength);
    return length;
    }

bool modbusTcpConnect(const char *host, int port, uint8_t unitId) {
    ESP_LOGI(TAG, "Connecting to Modbus TCP server at %s:%d", host, port);
    return client.connect(host, port);
    }

void modbusTcpDisconnect() {
    ESP_LOGI(TAG, "Disconnecting from Modbus TCP server");
    client.stop();
    }

static uint8_t modbus_error;
static uint16_t *modbusTcpRead(uint8_t unitId,uint8_t function, uint16_t startAddr, uint16_t quantity) {

    // modbus activities, including sending request, receiving response, and parsing response, can be expanded later to include more detailed error handling, retries, etc. as needed for robustness in real-world applications
    pixelBlink(10,10,0);

    uint8_t request[12];
    uint16_t respLength;
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

    // sends request for header
    sendRequest(request,sizeof(request));
 
    // read response, check for errors, extract register values into provided buffer, and add to json response for mqtt transmission, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    pixelBlink(10,10,0);
    uint16_t respsize=9+quantity*2; // expected response size based on quantity of registers requested, can be used for validation of response length as needed for robustness in real-world applications
    static uint8_t buffer[MODBUS_TCP_BUFFER];
    respLength=receiveResponse(buffer,respsize);
    if (respLength == 0) {
        modbus_error=0xe0; // custom error code for no response
        ESP_LOGE(TAG, "No Data received from Modbus TCP server");
        return NULL;
        }
   

    // basic validation of response header, can be expanded later to include more detailed validation and error handling as needed for robustness in real-world applications
    modbus_error=0;
    uint8_t *header=buffer;
    uint16_t recv_transaction = (header[0] << 8) | header[1];
    uint16_t protocol_id = (header[2] << 8) | header[3];
    uint16_t length = (header[4] << 8) | header[5];
    uint8_t recv_unit = header[6];

    if (recv_transaction != transactionId) {
        ESP_LOGW(TAG, "transaction id mismatch: expected %u got %u", transactionId, recv_transaction);
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
  
    // reads pdu data, checks for errors, and extracts register values into provided buffer, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    respLength-=7; // adjust length to account for header
    uint8_t *pdu=buffer+7; // pdu starts after unit id in response
    if (respLength == 0) {
        modbus_error=0xe1; // custom error code for no response
        ESP_LOGE(TAG, "No PDU received from Modbus TCP server");
        return NULL;
        }

    // error response has function code with MSB set and error code in byte 8 of response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (pdu[0] & 0x80) {
        ESP_LOGE(TAG, "Modbus error response received: function code 0x%02X, error code 0x%02X", pdu[0], pdu[1]);
        modbus_error=pdu[1]; // error code is in byte 8 of response
        return NULL;
        }
    
    //unexpected function code in response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (pdu[0] != function) {
        ESP_LOGE(TAG, "Unexpected function code in response: 0x%02X", pdu[0]);
        modbus_error=pdu[0]; // error code is in byte 8 of response
        return NULL;
    }

    if(pdu[1]!=quantity*2) {
        ESP_LOGE(TAG, "Unexpected byte count in response: expected %d got %d", quantity*2, pdu[1]);
        modbus_error=0xff; // custom error code for invalid response
        return NULL;
    }


    // return pointer to register values in response starting from count
    return (uint16_t*)(pdu+1);
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

    char jobjectid[BUFTINY];
    sprintf(jobjectid,"x%04x",start_address);

    // reads data from modbus tcp server, response is array of uint16_t, jsonAddValue will add as number, if want to add as string need to convert to string first
    ESP_LOGI(TAG, "Reading from Modbus TCP server: unit_id=%d, function=%d, start_address=%d, quantity=%d", unit_id, func, start_address, quantity);
    uint16_t *response=modbusTcpRead(unit_id , func, start_address, quantity);

    jsonAddArray(jobjectid);
    jsonAddValue(func);
    jsonAddValue(modbus_error);
    jsonAddValue(start_address);

    // get values for non null response, if response is null, it means there was an error, modbus_error variable will have error code, if response is not null, modbus_error should be 0
    for (uint16_t i = 0; response!=NULL && i < quantity; ++i) {
        ESP_LOGI(TAG, "Read register %u: %u", start_address + i, response[i]);
        jsonAddValue(response[i]);
        }

    jsonClose();

    return response;
}

#else
bool modbusTcpConnect(const char *host, int port, uint8_t unitId) {
    ESP_LOGW(TAG, "Modbus TCP client functionality is not available on this platform");
    return false;
    }

void modbusTcpDisconnect() {
    ESP_LOGW(TAG, "Modbus TCP client functionality is not available on this platform");
    }

uint16_t *modbusTcpReadJson(uint8_t unit_id, uint8_t func, uint16_t start_address, uint16_t quantity) {
    ESP_LOGW(TAG, "Modbus TCP client functionality is not available on this platform");
    return NULL;
    }   

#endif
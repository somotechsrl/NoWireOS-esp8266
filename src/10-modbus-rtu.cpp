#include <Arduino.h>
#include "main.h"
#include "10-encoder.h"

// Uses Software Serial 
#ifdef CUBE_CELL
#include "softSerial.h"
softSerial modbusSerial(GPIO5, GPIO6);
#else
#include <SoftwareSerial.h>
SoftwareSerial modbusSerial(10, 11);
#endif

#define TAG "MBRTU"
#define MODBUS_RTU_BUFFER  256

// Modbus RTU Master Implementation
#define MODBUS_BAUDRATE 9600
#define MODBUS_TIMEOUT 1000

static uint16_t transactionId;
static uint16_t modbus_timeout_ms=MODBUS_TIMEOUT; 

uint16_t calculateCRC(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static bool receiveResponse(uint8_t *rxBuffer, uint8_t maxLength) {
    
    unsigned long startTime = millis();
    uint8_t length = 0;
    
    while (millis() - startTime < modbus_timeout_ms && length < maxLength) {
        if (modbusSerial.available()) {
            rxBuffer[length++] = modbusSerial.read();
            if (length > 250) return false;
        }
    }
    
    if (length < 5) return false;
    
    uint16_t crc = calculateCRC(rxBuffer, length - 2);
    uint16_t rxCRC = (rxBuffer[length - 1] << 8) | rxBuffer[length - 2];

    ESP_LOGI(TAG,"Received %u of %u bytes from Modbus TCP server", length, maxLength);
    
    return (crc == rxCRC);
}

static uint8_t modbus_error;
static uint16_t receiveResponse(uint8_t* response, uint16_t maxLength) {

    uint16_t length=0;
    unsigned long timeout = millis() + modbus_timeout_ms;
        
    // includes crc
    maxLength+=2;

    while (millis() < timeout && length < maxLength) {
        if (modbusSerial.available()) {
            response[length++]=modbusSerial.read();
            }
        }

    // ot what expected
    if (length!=maxLength) {
        modbus_error=0xe0; // custom error code for no response
        ESP_LOGE(TAG, "Wrong data size: expected %u, received %u", maxLength, length);
        return 0;
        }
    
    uint16_t crc = calculateCRC(response, length - 2);
    uint16_t rxCRC = (response[length - 1] << 8) | response[length - 2];
    if(crc != rxCRC) {
        modbus_error=0xe2; // custom error code for CRC mismatch    
        ESP_LOGE(TAG,"Wrong CRC: calculated 0x%04X, received 0x%04X", crc, rxCRC);
        return 0;
        }

    ESP_LOGI(TAG,"Received %u of %u bytes from Modbus TCP server", length, maxLength);
    return length-2; // return length without crc
    }

static uint16_t *modbusRTURead(uint8_t slaveId,uint8_t function, uint16_t startAddr, uint16_t quantity) {

    // modbus activities, including sending request, receiving response, and parsing response, can be expanded later to include more detailed error handling, retries, etc. as needed for robustness in real-world applications
    pixelBlink(10,10,0);

    uint8_t request[8];
    uint16_t respLength;
    transactionId++;
   
    request[0] = slaveId;
    request[1] = function;
    request[2] = (startAddr >> 8) & 0xFF;
    request[3] = startAddr & 0xFF;
    request[4] = (quantity >> 8) & 0xFF;
    request[5] = quantity & 0xFF;
    
    uint16_t crc = calculateCRC(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    // sends request for header
    modbusSerial.write(request, 8);

    // read response, check for errors, extract register values into provided buffer, and add to json response for mqtt transmission, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    pixelBlink(10,10,0);
    // 3 byte header+quantity*2 byte data
    // crc is excluded by receiveReposnse function, so we expect length to be 3+quantity*2
    uint16_t respsize=3+quantity*2; 
    // expected response size based on quantity of registers requested, can be used for validation of response length as needed for robustness in real-world applications
    static uint8_t buffer[MODBUS_RTU_BUFFER];
    respLength=receiveResponse(buffer,respsize);
    if (respLength == 0 || respLength != respsize) {
        modbus_error=0xe0; // custom error code for no response
        ESP_LOGE(TAG, "No Data received from Modbus TCP server");
        return NULL;
        }
   
    // basic validation of response header, can be expanded later to include more detailed validation and error handling as needed for robustness in real-world applications
    modbus_error=0;
    uint8_t *header=buffer;
    uint8_t recv_slave = header[0];
    uint8_t recv_function = header[1];
    uint16_t recv_length = header[2];

    if (recv_slave != slaveId) {
        ESP_LOGW(TAG, "unexpected slave id %u", recv_slave);
        modbus_error=0xfc; // custom error code for slave id mismatch
        return NULL;
    }

    // reads pdu data, checks for errors, and extracts register values into provided buffer, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    respLength-=3; // adjust length to account for header
    uint8_t *pdu=buffer+3; // pdu starts after header in response
    if (respLength == 0) {
        modbus_error=0xe1; // custom error code for no response
        ESP_LOGE(TAG, "No PDU received from Modbus TCP server");
        return NULL;
        }

    // error response has function code with MSB set and error code in byte 8 of response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (recv_function & 0x80) {
        ESP_LOGE(TAG, "Modbus error response received: function code 0x%02X, error code 0x%02X", pdu[0], pdu[1]);
        modbus_error=pdu[1]; // error code is in byte 8 of response
        return NULL;
        }
    
    //unexpected function code in response, can be expanded later to include more detailed error handling as needed for robustness in real-world applications
    if (recv_function != function) {
        ESP_LOGE(TAG, "Unexpected function code in response: 0x%02X", recv_function);
        modbus_error=recv_function; // error code is in byte 8 of response
        return NULL;
    }

    if(recv_length!=quantity*2) {
        ESP_LOGE(TAG, "Unexpected byte count in response: expected %d got %d", quantity*2, recv_length);
        modbus_error=0xff; // custom error code for invalid response
        return NULL;
    }


    // return pointer to register values in response starting from count
    return (uint16_t*)(pdu);
    }

void modbusRTUInit(uint32_t baudrate) {
    modbusSerial.begin(baudrate);
    }

// Default entry for modbus tcp client task, will be called by modbus client task loop for each call in config
uint16_t *modbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity) {

    char jobjectid[BUFTINY];
    sprintf(jobjectid,"x%04x",start_address);

    // reads data from modbus tcp server, response is array of uint16_t, jsonAddValue will add as number, if want to add as string need to convert to string first
    ESP_LOGI(TAG, "Reading from Modbus TCP server: slave_id=%d, function=%d, start_address=%d, quantity=%d", slave_id, func, start_address, quantity);
    uint16_t *response=modbusRTURead(slave_id , func, start_address, quantity);

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


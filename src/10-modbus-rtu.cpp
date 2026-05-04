
#include <Arduino.h>
#include "main.h"
#include "10-encoder.h"
#define TAG "MBRTU"

#ifdef MODBUS_RTU

// Uses Software Serial 
#ifdef CUBE_CELL
#include "softSerial.h"
softSerial modbusSerial(GPIO4, GPIO5);
#endif
#ifdef ESP8266
#include <SoftwareSerial.h>
#ifdef ARDUINO_ESP8266_ESP01
// really not used....
SoftwareSerial modbusSerial(0, 0);
#else
SoftwareSerial modbusSerial(D5, D6); // RX, TX pins for Modbus
#endif
#endif
#ifdef ESP32
// esp32 uses always serial1
#include <HardwareSerial.h>
HardwareSerial modbusSerial(1);
#define TXD1 17
#define RXD1 18
#define TXD1EN 21 // DE pin for RS485, can be changed as needed for different hardware configurations
#endif

#define MODBUS_RTU_BUFFER  256

// Modbus RTU Master Implementation
#define MODBUS_BAUDRATE 9600
#define MODBUS_TIMEOUT 2000

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

static uint8_t modbus_error;
static uint8_t receive[MODBUS_RTU_BUFFER];
static uint16_t receiveResponse(uint16_t maxLength) {

    uint16_t length=0;
    unsigned long timeout = millis() + modbus_timeout_ms;
        
    // includes crc
    maxLength+=2;

    memset(receive, 0, sizeof(receive));
    while (millis() < timeout && length < maxLength) {
        if (modbusSerial.available()) {
            receive[length++]=modbusSerial.read();
            }
        }

    // ot what expected
    if (length!=maxLength) {
        modbus_error=0xe0; // custom error code for no response
        ESP_LOGE(TAG, "Wrong data size: expected %u, received %u", maxLength, length);
        return 0;
        }
    
    uint16_t crc = calculateCRC(receive, length - 2);
    uint16_t rxCRC = (receive[length - 1] << 8) | receive[length - 2];
    if(crc != rxCRC) {
        modbus_error=0xe2; // custom error code for CRC mismatch    
        ESP_LOGE(TAG,"Wrong CRC: calculated 0x%04X, received 0x%04X", crc, rxCRC);
        return 0;
        }

    return length-2; // return length without crc
    }

static uint8_t *modbusRTURead(uint8_t slaveId,uint8_t function, uint16_t startAddr, uint16_t quantity) {

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
    
    // cleanup garbage?
    modbusSerial.flush();
    //while (modbusSerial.available()) modbusSerial.read();

// for RS485, set DE pin high to enable transmission, can be adjusted as needed for different hardware configurations
#ifdef ESP32
    modbusSerial.setPins(-1,-1,-1,TXD1EN); // set DE pin for RS485, can be changed as needed for different hardware configurations
    //modbusSerial.setMode(SERIAL_RS485_HALF_DUPLEX); // set half duplex mode for RS485, can be changed as needed for different hardware configurations
    #endif

    // sends request for header
    modbusSerial.write(request, 8);
    //while(!modbusSerial.txdone()) { /* Wait */ } // Wait for hardware to finish

#ifdef ESP32
    //digitalWrite(RS485_DE, LOW);
#endif

    // read response, check for errors, extract register values into provided buffer, and add to json response for mqtt transmission, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    pixelBlink(10,10,0);
    // 3 byte header+quantity*2 byte data
    // crc is excluded by receiveReposnse function, so we expect length to be 3+quantity*2
    uint16_t respsize=3+quantity*2; 
    // expected response size based on quantity of registers requested, can be used for validation of response length as needed for robustness in real-world applications
    respLength=receiveResponse(respsize);
    if (respLength != respsize) {
        modbus_error=0xe0; // custom error code for no response
        ESP_LOGE(TAG, "Wrong data size: expected %u, received %u", respsize, respLength);
        return NULL;
        }
   
    // basic validation of response header, can be expanded later to include more detailed validation and error handling as needed for robustness in real-world applications
    modbus_error=0;
    uint8_t recv_slave      = receive[0];
    uint8_t recv_function   = receive[1];
    uint8_t recv_length     = receive[2];

    if (recv_slave != slaveId) {
        ESP_LOGW(TAG, "unexpected slave id %u", recv_slave);
        modbus_error=0xfc; // custom error code for slave id mismatch
        return NULL;
    }

    // reads pdu data, checks for errors, and extracts register values into provided buffer, can be expanded later to include error handling, retries, etc. as needed for robustness in real-world applications
    respLength-=3; // adjust length to account for header
    uint8_t *pdu=receive+3; // pdu starts after header in response
    if (respLength == 0) {
        modbus_error=0xe1; // custom error code for no response
        ESP_LOGE(TAG, "No PDU received from Modbus RTU server");
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
    

    return pdu;
    }

void modbusRTUInit(uint32_t baudrate) {
    ESP_LOGI(TAG, "Initializing Modbus RTU with baudrate: %u", baudrate);
#ifdef ESP32
    modbusSerial.begin(baudrate,SERIAL_8N1,RXD1,TXD1); // RX, TX pins for Modbus, can be changed as needed for different hardware configurations
#else
    modbusSerial.begin(baudrate); // Start software serial for Modbus RTU, can be changed as needed for different hardware configurations
#endif
    }

// Default entry for modbus rtu client task, will be called by modbus client task loop for each call in config
void modbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity) {

    char jobjectid[BUFTINY];
    sprintf(jobjectid,"x%04x",start_address);

    // reads data from modbus rtu server, response is array of uint16_t, jsonAddValue will add as number, if want to add as string need to convert to string first
    ESP_LOGI(TAG, "Reading from Modbus RTU server: slave_id=%d, function=%d, start_address=%d, quantity=%d", slave_id, func, start_address, quantity);
    uint8_t *regs=modbusRTURead(slave_id , func, start_address, quantity);

    jsonAddArray(jobjectid);
    jsonAddValue(func);
    jsonAddValue(start_address);
    jsonAddValue(modbus_error);

    // get values for non null response, if response is null, it means there was an error, modbus_error variable will have error code, if response is not null, modbus_error should be 0
    for (uint8_t i = 0; regs!=NULL && i < quantity; i++) {
        uint16_t reg_value = (regs[i*2] << 8) | regs[i*2 + 1];
        ESP_LOGI(TAG, "Read register %u: %u", start_address + i, reg_value);
        jsonAddValue(reg_value);
        }

    jsonClose();
    }

#else

void modbusRTUInit(uint32_t baudrate) {
    ESP_LOGW(TAG, "Modbus RTU not supported on this platform");
    }

// Default entry for modbus rtu client task, will be called by modbus client task loop for each call in config
void modbusRTUReadJson(uint8_t slave_id, uint8_t func, uint16_t start_address, uint16_t quantity) {
    ESP_LOGW(TAG, "Modbus RTU not supported on this platform");
}

#endif
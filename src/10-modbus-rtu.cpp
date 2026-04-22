#include <Arduino.h>

#ifdef CUBE_CELL
#include "softSerial.h"
SoftwareSerial modbusSerial(GPIO5, GPIO6);
#else
#include <SoftwareSerial.h>
SoftwareSerial modbusSerial(10, 11);
#endif

class ModbusRTU {
private:
    static const uint8_t MAX_BUFFER = 256;
    uint8_t slaveID;
    uint8_t buffer[MAX_BUFFER];
    
public:
    ModbusRTU(uint8_t id) : slaveID(id) {
        modbusSerial.begin(9600);
    }
    
    void begin(long baudrate) {
        modbusSerial.begin(baudrate);
    }
    
    bool readCoils(uint8_t addr, uint8_t quantity) {
        uint8_t frame[8];
        frame[0] = slaveID;
        frame[1] = 0x01; // Read Coils
        frame[2] = (addr >> 8) & 0xFF;
        frame[3] = addr & 0xFF;
        frame[4] = 0x00;
        frame[5] = quantity;
        
        uint16_t crc = calculateCRC(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        
        return sendFrame(frame, 8);
    }
    
    bool readHoldingRegisters(uint8_t addr, uint8_t quantity) {
        uint8_t frame[8];
        frame[0] = slaveID;
        frame[1] = 0x03; // Read Holding Registers
        frame[2] = (addr >> 8) & 0xFF;
        frame[3] = addr & 0xFF;
        frame[4] = 0x00;
        frame[5] = quantity;
        
        uint16_t crc = calculateCRC(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        
        return sendFrame(frame, 8);
    }
    
    bool writeRegister(uint8_t addr, uint16_t value) {
        uint8_t frame[8];
        frame[0] = slaveID;
        frame[1] = 0x06; // Write Single Register
        frame[2] = (addr >> 8) & 0xFF;
        frame[3] = addr & 0xFF;
        frame[4] = (value >> 8) & 0xFF;
        frame[5] = value & 0xFF;
        
        uint16_t crc = calculateCRC(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        
        return sendFrame(frame, 8);
    }
    
    uint16_t getValue(uint8_t index) {
        return (buffer[index] << 8) | buffer[index + 1];
    }
    
private:
    bool sendFrame(uint8_t* frame, uint8_t length) {
        modbusSerial.write(frame, length);
        return true;
    }
    
    uint16_t calculateCRC(uint8_t* data, uint8_t length) {
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
};

ModbusRTU modbus(1); // Slave ID = 1

void xxsetup() {
    Serial.begin(115200);
    modbus.begin(9600);
}

void xxloop() {
    // Example: Read holding registers from address 0, quantity 10
    modbus.readHoldingRegisters(0, 10);
    delay(1000);
}
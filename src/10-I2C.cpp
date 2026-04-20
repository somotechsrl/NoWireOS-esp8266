#include <Wire.h>

class I2CInterface {
private:
    uint8_t address;
    TwoWire* wire;

public:
    I2CInterface(const String& config) {
        parseConfig(config);
    }

    void parseConfig(const String& config) {
        // Format: "ADDRESS:0x68,CLOCK:400000"
        int addrPos = config.indexOf("ADDRESS:");
        int clockPos = config.indexOf("CLOCK:");

        if (addrPos != -1) {
            int commaPos = config.indexOf(',', addrPos);
            String addrStr = config.substring(addrPos + 8, commaPos == -1 ? config.length() : commaPos);
            address = (uint8_t)strtol(addrStr.c_str(), nullptr, 16);
        }

        if (clockPos != -1) {
            String clockStr = config.substring(clockPos + 6);
            uint32_t clockSpeed = (uint32_t)strtol(clockStr.c_str(), nullptr, 10);
            Wire.setClock(clockSpeed);
        }

        wire = &Wire;
    }

    void begin() {
        wire->begin();
    }

    uint8_t write(const uint8_t* data, uint8_t length) {
        wire->beginTransmission(address);
        wire->write(data, length);
        return wire->endTransmission();
    }

    uint8_t read(uint8_t* buffer, uint8_t length) {
        wire->requestFrom(address, length);
        for (int i = 0; i < length && wire->available(); i++) {
            buffer[i] = wire->read();
        }
        return wire->available();
    }

    uint8_t getAddress() const {
        return address;
    }
};
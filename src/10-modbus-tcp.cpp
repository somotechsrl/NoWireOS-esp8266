#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Modbus TCP Client for ESP8266
class ModbusTCPClient {
private:
    WiFiClient client;
    IPAddress serverIP;
    uint16_t port;
    uint16_t transactionID;

    // Helper to send MBAP header + PDU
    bool sendRequest(uint8_t slaveID, uint8_t functionCode, uint16_t startAddr, uint16_t quantity, uint8_t* data = nullptr, uint16_t dataLen = 0) {
        uint8_t request[260]; // Max size
        uint16_t length = 6 + dataLen; // MBAP + PDU
        request[0] = (transactionID >> 8) & 0xFF;
        request[1] = transactionID & 0xFF;
        request[2] = 0x00;
        request[3] = 0x00;
        request[4] = (length >> 8) & 0xFF;
        request[5] = length & 0xFF;
        request[6] = slaveID;
        request[7] = functionCode;
        request[8] = (startAddr >> 8) & 0xFF;
        request[9] = startAddr & 0xFF;
        request[10] = (quantity >> 8) & 0xFF;
        request[11] = quantity & 0xFF;
        if (data) memcpy(&request[12], data, dataLen);
        client.write(request, 12 + dataLen);
        transactionID++;
        return true;
    }

    // Helper to receive response
    bool receiveResponse(uint8_t* buffer, size_t& len) {
        if (client.available() < 8) return false;
        client.readBytes(buffer, 8);
        uint16_t respLength = (buffer[4] << 8) | buffer[5];
        if (client.available() < respLength) return false;
        client.readBytes(&buffer[8], respLength);
        len = 8 + respLength;
        return true;
    }

public:
    ModbusTCPClient(IPAddress ip, uint16_t p) : serverIP(ip), port(p), transactionID(0) {}

    bool connect() {
        return client.connect(serverIP, port);
    }

    void disconnect() {
        client.stop();
    }

    // Read Coils (0x01)
    bool readCoils(uint8_t slaveID, uint16_t startAddr, uint16_t numCoils, bool* coils) {
        if (!sendRequest(slaveID, 0x01, startAddr, numCoils)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x01) return false;
        uint8_t byteCount = response[8];
        for (uint16_t i = 0; i < numCoils; i++) {
            uint8_t byteIndex = i / 8;
            uint8_t bitIndex = i % 8;
            coils[i] = (response[9 + byteIndex] & (1 << bitIndex)) != 0;
        }
        return true;
    }

    // Read Discrete Inputs (0x02)
    bool readDiscreteInputs(uint8_t slaveID, uint16_t startAddr, uint16_t numInputs, bool* inputs) {
        if (!sendRequest(slaveID, 0x02, startAddr, numInputs)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x02) return false;
        uint8_t byteCount = response[8];
        for (uint16_t i = 0; i < numInputs; i++) {
            uint8_t byteIndex = i / 8;
            uint8_t bitIndex = i % 8;
            inputs[i] = (response[9 + byteIndex] & (1 << bitIndex)) != 0;
        }
        return true;
    }

    // Read Holding Registers (0x03)
    bool readHoldingRegisters(uint8_t slaveID, uint16_t startAddr, uint16_t numRegs, uint16_t* regs) {
        if (!sendRequest(slaveID, 0x03, startAddr, numRegs)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x03) return false;
        uint8_t byteCount = response[8];
        for (uint16_t i = 0; i < numRegs; i++) {
            regs[i] = (response[9 + 2*i] << 8) | response[10 + 2*i];
        }
        return true;
    }

    // Read Input Registers (0x04)
    bool readInputRegisters(uint8_t slaveID, uint16_t startAddr, uint16_t numRegs, uint16_t* regs) {
        if (!sendRequest(slaveID, 0x04, startAddr, numRegs)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x04) return false;
        uint8_t byteCount = response[8];
        for (uint16_t i = 0; i < numRegs; i++) {
            regs[i] = (response[9 + 2*i] << 8) | response[10 + 2*i];
        }
        return true;
    }

    // Write Single Coil (0x05)
    bool writeSingleCoil(uint8_t slaveID, uint16_t addr, bool value) {
        uint16_t data = value ? 0xFF00 : 0x0000;
        uint8_t dataBytes[2] = {(data >> 8) & 0xFF, data & 0xFF};
        if (!sendRequest(slaveID, 0x05, addr, 0, dataBytes, 2)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x05) return false;
        return true;
    }

    // Write Single Register (0x06)
    bool writeSingleRegister(uint8_t slaveID, uint16_t addr, uint16_t value) {
        uint8_t dataBytes[2] = {(value >> 8) & 0xFF, value & 0xFF};
        if (!sendRequest(slaveID, 0x06, addr, 0, dataBytes, 2)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x06) return false;
        return true;
    }

    // Write Multiple Coils (0x0F)
    bool writeMultipleCoils(uint8_t slaveID, uint16_t startAddr, uint16_t numCoils, bool* coils) {
        uint8_t byteCount = (numCoils + 7) / 8;
        uint8_t data[257]; // Max 256 bytes + byteCount
        data[0] = byteCount;
        for (uint16_t i = 0; i < byteCount; i++) {
            data[1 + i] = 0;
            for (uint8_t j = 0; j < 8; j++) {
                uint16_t coilIndex = i * 8 + j;
                if (coilIndex < numCoils && coils[coilIndex]) {
                    data[1 + i] |= (1 << j);
                }
            }
        }
        if (!sendRequest(slaveID, 0x0F, startAddr, numCoils, data, 1 + byteCount)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x0F) return false;
        return true;
    }

    // Write Multiple Registers (0x10)
    bool writeMultipleRegisters(uint8_t slaveID, uint16_t startAddr, uint16_t numRegs, uint16_t* regs) {
        uint8_t byteCount = numRegs * 2;
        uint8_t data[257];
        data[0] = byteCount;
        for (uint16_t i = 0; i < numRegs; i++) {
            data[1 + 2*i] = (regs[i] >> 8) & 0xFF;
            data[2 + 2*i] = regs[i] & 0xFF;
        }
        if (!sendRequest(slaveID, 0x10, startAddr, numRegs, data, 1 + byteCount)) return false;
        uint8_t response[260];
        size_t len;
        if (!receiveResponse(response, len) || response[7] != 0x10) return false;
        return true;
    }
};
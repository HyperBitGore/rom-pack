#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

class Buffer {
    private:
        std::vector<uint8_t> data;
    public:
    size_t offset = 0;
    Buffer () {

    }
    Buffer (const Buffer& b) {
        this->data = b.data;
        this->offset = b.offset;
    }
    Buffer (std::vector<uint8_t> buffer) {
        data = buffer;
    }
    // write
    void addEightByte (uint64_t data) {
        this->data.push_back(data & 0xFF);
        this->data.push_back((data >> 8) & 0xFF);
        this->data.push_back((data >> 16) & 0xFF);
        this->data.push_back((data >> 24) & 0xFF);
        this->data.push_back((data >> 32) & 0xFF);
        this->data.push_back((data >> 40) & 0xFF);
        this->data.push_back((data >> 48) & 0xFF);
        this->data.push_back((data >> 56) & 0xFF);
    }

    void addFourByte (uint32_t data) {
        this->data.push_back(data & 0xFF);
        this->data.push_back((data >> 8) & 0xFF);
        this->data.push_back((data >> 16) & 0xFF);
        this->data.push_back((data >> 24) & 0xFF);
    }
    void addTwoByte (uint16_t data) {
        this->data.push_back(data & 0xFF);
        this->data.push_back((data >> 8) & 0xFF);
    }
    void addByte (uint8_t data) {
        this->data.push_back(data);
    }
    void addString (std::string str) {
        addFourByte(str.size());
        for (auto& i : str) {
            this->data.push_back(i);
        }
    }
    std::vector<uint8_t>& getData () {
        return data;
    }
    // read
    uint32_t readFourByte () {
        uint32_t out = data[offset++];
        out |= data[offset++] << 8;
        out |= data[offset++] << 16;
        out |= data[offset++] << 24;
        return out;
    }
    uint16_t readTwoByte () {
        uint16_t out = data[offset++];
        out |= data[offset++] << 8;
        return out;
    }
    uint8_t readByte () {
        return data[offset++];
    }
    std::string readString () {
        uint32_t size = readFourByte();
        std::string str;
        for (size_t i = 0; i < size; i++, offset++) {
            str.push_back(data[offset]);
        }
        return str;
    }
    uint8_t& operator[](size_t i) {
        if (i < data.size()) {
            return data[i];
        }
        throw std::runtime_error("Accessing data out of bounds in buffer!");
    }
    size_t size () {
        return data.size();
    }
};
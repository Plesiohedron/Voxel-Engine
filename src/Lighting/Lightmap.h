#pragma once

#define CHUNK_WIDTH  16
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH  16

#include <stdint.h>

class Lightmap {
private:
    uint16_t* map_;

public:
    Lightmap(uint16_t* map);

    inline uint8_t Get(int x, int y, int z, uint8_t channel) const {
        return (map_[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> (channel << 2)) & 0xF;
    }
    inline uint8_t GetR(int x, int y, int z) const {
        return map_[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] & 0xF;
    }
    inline uint8_t GetG(int x, int y, int z) const {
        return (map_[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 4) & 0xF;
    }
    inline uint8_t GetB(int x, int y, int z) const {
        return (map_[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 8) & 0xF;
    }
    inline uint8_t GetS(int x, int y, int z) const {
        return (map_[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 12) & 0xF;
    }

    inline void Set(int x, int y, int z, int channel, uint8_t value) {
        const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
        map_[index] = (map_[index] & (0xFFFF & (~(0xF << (channel * 4))))) | (value << (channel << 2));
    }
    inline void SetR(int x, int y, int z, uint8_t value) {
        const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
        map_[index] = (map_[index] & 0xFFF0) | value;
    }
    inline void setG(int x, int y, int z, uint8_t value) {
        const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
        map_[index] = (map_[index] & 0xFF0F) | (value << 4);
    }
    inline void SetB(int x, int y, int z, uint8_t value) {
        const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
        map_[index] = (map_[index] & 0xF0FF) | (value << 8);
    }
    inline void SetS(int x, int y, int z, uint8_t value) {
        const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
        map_[index] = (map_[index] & 0x0FFF) | (value << 12);
    }
};
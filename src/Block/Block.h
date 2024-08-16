#pragma once

#include <cstdint>

struct Block {
    uint8_t texture_id[6];
    uint8_t r, g, b;
};

namespace Blocks {
extern Block blocks[256];
};
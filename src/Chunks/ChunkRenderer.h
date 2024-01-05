#pragma once

#include "Chunk.h"

#include <vector>
#include <glm/glm.hpp>

class ChunkRenderer {
    static const int CHUNK_W = 16;
    static const int CHUNK_H = 256;
    static const int CHUNK_D = 16;
    static const int CHUNK_VOL = CHUNK_W * CHUNK_H * CHUNK_D;

    static const int VERTEX_SIZE = 6;
public:
    static void render(Chunk& chunk, const Chunk** neighbouringСhunks);
};
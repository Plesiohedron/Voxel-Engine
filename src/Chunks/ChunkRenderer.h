#pragma once

#include "Chunk.h"

#define VERTEX_SIZE 3
#define VERTICES_COUNT_PER_SQUARE 4
#define INDEXES_COUNT_PER_SQUARE 6

class ChunkRenderer {
public:
    static void render(Chunk& chunk);
};
#pragma once

#include "Chunk.h"

#include <vector>
#include <unordered_map>

#define VERTEX_SIZE 3
#define VERTICES_COUNT_PER_SQUARE 4
#define INDEXES_COUNT_PER_SQUARE 6

class ChunkRenderer {
private:
    static std::unordered_map<uint8_t, bool> dataPlaneUpdate1;
    static std::unordered_map<uint8_t, bool> dataPlaneUpdate2;
    static std::unordered_map<uint8_t, bool> dataPlaneUpdate3;

    static void GreedyMesh(const Chunk& chunk, std::vector<GLushort>& vertices, std::vector<GLushort>& indexes);
public:
    static void Render(Chunk& chunk, std::vector<GLushort>& vertices, std::vector<GLushort>& indexes);
};
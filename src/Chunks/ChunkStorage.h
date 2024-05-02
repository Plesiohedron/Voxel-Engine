#pragma once

#include "Voxel.h"
#include "Chunk.h"

class ChunkStorage {
    friend class Engine;

public:
    std::vector<Chunk*> chunks;
    glm::mat4 model;

    const int CHUNKS_COUNT_IN_HEIGHT = 16;

    int rendering_radius;
    glm::ivec3 rendering_center;
    static glm::ivec3 storage_sizes;

private:
    ChunkStorage(const int radius, const glm::ivec3& center);
    ~ChunkStorage();

    ChunkStorage(const ChunkStorage&) = delete;
    ChunkStorage(ChunkStorage&&) = delete;
    ChunkStorage& operator=(const ChunkStorage&) = delete;
    ChunkStorage& operator=(ChunkStorage&&) = delete;
};
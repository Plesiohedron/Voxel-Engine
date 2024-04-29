#pragma once

#include "Voxel.h"

#include <GL/glew.h>

#include <vector>

#define CHUNK_W 16
#define CHUNK_H 16
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)
#define ATTRIBUTES_COUNT 3

class Chunk {
public:
    Voxel* voxels;

    std::vector<std::vector<std::vector<std::vector<std::vector<uint32_t>>>>> meshData;

    Chunk();
    ~Chunk();
};

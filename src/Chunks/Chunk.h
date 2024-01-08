#pragma once

#include "Voxel.h"

#include <GL/glew.h>

#define CHUNK_W 16
#define CHUNK_H 16
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)
#define ATTRIBUTES_COUNT 3

class Chunk {
public:
    unsigned currentVerticesCount = 0;
    unsigned currentIndexesCount = 0;

    Voxel* voxels;

    GLushort* vertices;
    GLushort* indexes;

    Chunk();
    ~Chunk();
};

#pragma once

#include "Voxel.h"

#include <vector>

class Chunk {
    static const int WIDTH = 16;
    static const int HEIGHT = 16;
    static const int DEPTH = 16;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

    static const int ATTRIBUTES_COUNT = 6;
public:
    unsigned currentVerticesCount = 0;
    unsigned currentIndexesCount = 0;

    Voxel* voxels;

    float* vertices;
    unsigned* indexes;

    Chunk();
    ~Chunk();
};

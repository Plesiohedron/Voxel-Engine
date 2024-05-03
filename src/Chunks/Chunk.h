#pragma once

#include <unordered_map>
#include <memory>
#include <cmath>

#include "Voxel.h"
#include "../GL/VAO.h"
#include "Multi-D Dynamic Array/MultiArray.h"

class Chunk {
    friend class ChunkStorage;

private:
    MultiArray<uint32_t> mesh_data_;
    Voxel* voxels_;

public:
    unsigned short int* vertices;
    unsigned short int* indexes;

    unsigned int vertices_array_size = 0;
    unsigned int indexes_array_size = 0;

    int global_coordinate_X;
    int global_coordinate_Y;
    int global_coordinate_Z;

    static const int VERTEX_ATTRIBUTES_COUNT = 3;
    static const int VERTICES_COUNT_PER_SQUARE = 4;
    static const int INDEXES_COUNT_PER_SQUARE = 6;

    static const int WIDTH = 16;
    static const int HEIGHT = 16;
    static const int DEPTH = 16;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

private:
    Chunk(const glm::ivec3& coordinates);
    Chunk(const Chunk&) = delete;
    ~Chunk();

public:
    void Render();
    void Culling();
    void GreedyMeshing();
};

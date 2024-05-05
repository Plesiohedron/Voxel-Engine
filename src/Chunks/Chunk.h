#pragma once

#include "Voxel.h"
#include "../GL/SpecialVAO.h"
#include "Multi-D Dynamic Array/MultiArray.h"

#include <glm/glm.hpp>

class Chunk {
    friend class Chunks;

private:
    MultiArray<uint32_t> mesh_data_;
    Voxel* voxels_;

public:
    int global_coordinate_X;
    int global_coordinate_Y;
    int global_coordinate_Z;

    static const int WIDTH = 16;
    static const int HEIGHT = 16;
    static const int DEPTH = 16;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

    static const int VERTEX_ATTRIBUTES_COUNT = 3;
    static const int VERTICES_COUNT_PER_SQUARE = 4;
    static const int INDEXES_COUNT_PER_SQUARE = 6;

    // Change in the future
    static const int MAXIMUM_VOXEL_FACES_COUNT = 500;

private:
    Chunk(const glm::ivec3& coordinates);
    Chunk(const Chunk&) = delete;
    ~Chunk();

public:
    void Render(GLushort* vertex_data_section, GLushort* index_data_section, GL::IndirectCommand* indirect_command_data_section);
    void Culling();
    unsigned int GreedyMeshing(GLushort* vertex_data_section, GLushort* index_data_section);
};

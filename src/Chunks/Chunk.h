#pragma once

#include <unordered_map>
#include <memory>

#include "../GL/VAO.h"
#include "Voxel.h"
//#include "Multi-D Dynamic Arrays/MultiArray.h"

class Chunk {
private:


    std::unordered_map<uint8_t, bool> dataPlaneUpdate1;
    std::unordered_map<uint8_t, bool> dataPlaneUpdate2;
    std::unordered_map<uint8_t, bool> dataPlaneUpdate3;

public:
    int global_coordinate_X;
    int global_coordinate_Y;
    int global_coordinate_Z;

    int local_coordinate_X;
    int local_coordinate_Y;
    int local_coordinate_Z;

    Voxel* voxels;
    std::vector<std::vector<std::vector<std::vector<std::vector<uint32_t>>>>> mesh_data;

    std::vector<unsigned short int> vertices;
    std::vector<unsigned short int> indexes;

    std::unique_ptr<GL::VAO> VAO;

    static const int VERTEX_ATTRIBUTES_COUNT = 3;
    static const int VERTICES_COUNT_PER_SQUARE = 4;
    static const int INDEXES_COUNT_PER_SQUARE = 6;

    static const int WIDTH = 16;
    static const int HEIGHT = 16;
    static const int DEPTH = 16;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

    Chunk(const glm::ivec3& coordinates);
    Chunk(const Chunk&) = delete;
    ~Chunk();

    void Render();
    void Culling();
    void GreedyMeshing();
    void Draw(const GLenum primitive_type) const;
};

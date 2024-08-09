#pragma once

#include <vector>
#include <iostream>

struct Voxel {
    uint8_t id;
};

class Chunk {
public:
    int global_coordinate_X = 0;
    int global_coordinate_Y = 0;
    int global_coordinate_Z = 0;

    static const int DIRECTION_SIZE = 16;
    static const int DIRECTION_SIZE_P2 = 256;

    static const int WIDTH = 16;
    static const int HEIGHT = 16;
    static const int DEPTH = 16;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

    static const int VERTICES_COUNT_PER_SQUARE = 4;
    static const int INDEXES_COUNT_PER_SQUARE = 6;
    static const int FACES_COUNT_PER_CUBE = 6;

    int VOXEL_FACES_CAPACITY = 512;
    std::vector<uint64_t> vertex_data;
    std::vector<uint32_t> index_data;

    uint16_t* face_planes_[Chunk::FACES_COUNT_PER_CUBE];
    Voxel* voxels_;

private:
    void Culling(uint16_t(&X_rows)[Chunk::HEIGHT][Chunk::DEPTH], uint16_t(&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                 uint16_t(&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]);

    bool IsBlocked(int x, int y, int z);
    uint64_t AmbientOcclusion(int x, int y, int z, int direction);

public:
    Chunk();
    Chunk(const Chunk&) = delete;
    ~Chunk();

    void GreedyMesh();
};



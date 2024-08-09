#pragma once

#include "Voxel.h"

#include <vector>
#include <glm/glm.hpp>

class Chunk {
    friend class Chunks;

public:
    glm::ivec3 global_coordinates;
    glm::ivec3 local_coordinates;

    static const int DIRECTION_SIZE = 16;
    static const int DIRECTION_SIZE_P2 = 256;

    static const int WIDTH = DIRECTION_SIZE;
    static const int HEIGHT = DIRECTION_SIZE;
    static const int DEPTH = DIRECTION_SIZE;
    static const int VOLUME = WIDTH * HEIGHT * DEPTH;

    static const int VERTICES_COUNT_PER_SQUARE = 4;
    static const int INDEXES_COUNT_PER_SQUARE = 6;
    static const int FACES_COUNT_PER_CUBE = 6;

    static const int STARTING_VOXEL_FACES_CAPACITY = 64;

    int voxel_faces_size = 0;
    int voxel_faces_capacity = STARTING_VOXEL_FACES_CAPACITY;

    int vertex_data_size = 0;
    int vertex_data_capacity = STARTING_VOXEL_FACES_CAPACITY * VERTICES_COUNT_PER_SQUARE;
    uint64_t* vertex_data;

    bool is_modified;

private:
    static Chunk** chunk_storage_;
    uint16_t* face_planes_[Chunk::FACES_COUNT_PER_CUBE];
    Voxel* voxels_;

private:
    void Culling(uint16_t (&X_rows)[Chunk::HEIGHT][Chunk::DEPTH], uint16_t (&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                 uint16_t (&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]);
    void CullingChunksJoints();

    inline bool IsBlocked(int x, int y, int z);
    inline uint64_t AmbientOcclusion(int x, int y, int z, int direction);
    inline void PushBack(uint64_t vertex);

private:
    Chunk(const glm::ivec3& coordinates);
    Chunk(const Chunk&) = delete;
    ~Chunk();

public:
    void GreedyMesh();
};
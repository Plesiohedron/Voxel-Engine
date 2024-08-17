#pragma once

#include "Voxel.h"
#include "../Lighting/Lightmap.h"
#include "ChunkStorage.h"

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    uint32_t l0;
    uint32_t l1;
    uint32_t l2;
    uint32_t l3;
    uint32_t dimension;
};

class Chunk {
    friend class ChunkStorage;
    friend class LightProcessor;

public:
    // Lightmap defines
    static constexpr int DIRECTION_SIZE = 16;
    static constexpr int DIRECTION_SIZE_P2 = 256;

    static constexpr int WIDTH = DIRECTION_SIZE;
    static constexpr int HEIGHT = DIRECTION_SIZE;
    static constexpr int DEPTH = DIRECTION_SIZE;
    static constexpr int VOLUME = WIDTH * HEIGHT * DEPTH;

    static constexpr int VERTICES_COUNT_PER_SQUARE = 4;
    static constexpr int INDEXES_COUNT_PER_SQUARE = 6;
    static constexpr int FACES_COUNT_PER_CUBE = 6;

    static constexpr int STARTING_VOXEL_FACES_CAPACITY = 64;

    glm::ivec3 global_coordinates;
    glm::ivec3 local_coordinates;

    int voxel_faces_size = 0;
    int voxel_faces_capacity = STARTING_VOXEL_FACES_CAPACITY;

    int vertex_data_size = 0;
    int vertex_data_capacity = STARTING_VOXEL_FACES_CAPACITY * VERTICES_COUNT_PER_SQUARE;
    Vertex* vertex_data;

    bool is_modified;

private:
    Voxel* voxels_;
    Lightmap* lightmap_;
    uint16_t* face_planes_[Chunk::FACES_COUNT_PER_CUBE];

    const ChunkStorage* chunk_storage_;

private:
    void Culling(uint16_t (&X_rows)[Chunk::HEIGHT][Chunk::DEPTH],
                 uint16_t (&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                 uint16_t (&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]);
    void CullingChunksJoints();

    inline bool IsBlocked(int x, int y, int z) const;
    inline uint32_t Light(int x, int y, int z, int direction, int vertex) const;
    inline void PushBack(Vertex vertex);

    void GreedyMesh();

private:
    Chunk(const glm::ivec3& coordinates, Voxel* voxels, uint16_t* lightmap, uint16_t* face_planes, const ChunkStorage* chunk_storage);
    Chunk(const Chunk&) = delete;
    ~Chunk();
};

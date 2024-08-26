#pragma once

#include "Chunk.h"
#include "../Lighting/LightProcessor.h"
#include "../LineBatch/LineBatch.h"

#include <glm/glm.hpp>

class ChunkStorage {
    friend class Chunk;
    friend class Chunks;

public:
    glm::ivec3 sizes;
    glm::ivec3 rendering_center;
    int rendering_radius;
    int chunk_count;

    uint8_t selected_block = 1;

private:
    Voxel* voxels_;
    uint16_t* face_planes_;
    uint16_t* lightmaps_;
    Chunk** chunks_;

    LightProcessor* R;
    LightProcessor* G;
    LightProcessor* B;
    LightProcessor* S;

public:
    Voxel* GetVoxel(int x, int y, int z) const;                // global voxel coordinates
    void SetVoxel(int x, int y, int z, uint8_t block_id);      // global voxel coordinates
    uint8_t GetLight(int x, int y, int z, int channel) const;  // global voxel coordinates
    Chunk* GetChunk(int x, int y, int z) const;                // local chunk coordinates
    Chunk* GetChunkByVoxel(int x, int y, int z) const;         // global voxel coordinates

    void CaptureTarget(LineBatch& line_batch, const Camera& camera);
    void UpdateChanges();

private:
    ChunkStorage(const glm::ivec3& sizes);
    ~ChunkStorage();

    Voxel* RayCast(const glm::vec3& position, const glm::vec3& direction, float& ray_length, glm::vec3& end, glm::ivec3& normal, glm::vec3& iend) const;
    void FrustumRayCast(const glm::vec3& position, const glm::vec3& direction, float ray_length) const;
};

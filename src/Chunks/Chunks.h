#pragma once

#include "ChunkStorage.h"

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/SChunkVAO.h"
#include "../GL/SSBO.h"
#include "../Camera/Camera.h"

class Chunks {
    friend class Engine;

private:
    ChunkStorage storage_;

    glm::mat4* models_;
    GL::SChunkVAO** VAOs_;
    GLuint unified_EBO_;
    GL::SSBO matrices_SSBO_;

    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_;

    GLint uniform_texture_loc_;
    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;
    GLint uniform_model_index_loc_;

public:
    static const int UNIFIED_EBO_SIZE = Chunk::INDEXES_COUNT_PER_SQUARE * Chunk::FACES_COUNT_PER_CUBE * Chunk::VOLUME / 2;
    static const int CHUNK_COUNT_IN_HEIGHT = 16;

    bool debug_mode = false;

    glm::vec3 frustum_TL;
    glm::vec3 frustum_TR;
    glm::vec3 frustum_BR;
    glm::vec3 frustum_BL;

    float frustum_side_edge_length;
    float frustum_volume_sixed;

private:
    Chunks(const int radius, const glm::ivec3& center);
    Chunks(const Chunks&) = delete;
    ~Chunks();

public:
    void FrustumCulling(const glm::vec3& camera_position);
    void PollUpdates();
    void Draw(const Camera& camera) const;
};

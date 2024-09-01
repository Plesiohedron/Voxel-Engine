#pragma once

#include "ChunkStorage.h"

#include "../CL/ComputeProgram.h"
#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/SChunkVAO.h"
#include "../GL/SSBO.h"
#include "../Camera/Camera.h"

//#define FRUSTUM_CULLING_GPU

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

    #ifdef FRUSTUM_CULLING_GPU
    std::unique_ptr<CL::Program> program_;
    std::vector<float> input_buffer;
    std::vector<char> output_buffer;
    #endif

    std::queue<glm::ivec4> ACCA_queue_;

public:
    static const int UNIFIED_EBO_SIZE = Chunk::INDEXES_COUNT_PER_SQUARE * Chunk::FACES_COUNT_PER_CUBE * Chunk::VOLUME / 2;
    static const int CHUNK_COUNT_IN_HEIGHT = 16;

    bool debug_mode = false;

    glm::vec3 frustum_TL;
    glm::vec3 frustum_TR;
    glm::vec3 frustum_BR;
    glm::vec3 frustum_BL;

    glm::mat4 rotation = glm::mat4(1.0f);

    const float h_near = tan(glm::radians(90.0f) / 2);
    const float w_near = h_near * Events::window->GetAspect();

    float frustum_side_edge_length;

private:
    Chunks(const int radius, const glm::ivec3& center);
    Chunks(const Chunks&) = delete;
    ~Chunks();

public:
    void FrustumCulling(const glm::vec3& camera_position) const;
    void ACCA(const glm::vec3& camera_position);
    void PollUpdates();
    void Draw(const Camera& camera) const;
};

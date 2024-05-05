#pragma once

#include "Chunk.h"

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/SpecialVAO.h"
#include "../Camera/Camera.h"

#include <unordered_map>

class Chunks {
    friend class Engine;

private:
    glm::mat4* models_;

    Chunk** chunks_;

    std::unique_ptr<GL::SpecialVAO> unified_VAO_;
    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_;

    GLushort* unified_voxel_vertex_data_;
    GLushort* unified_voxel_index_data_;
    GL::IndirectCommand* unified_voxel_indirect_command_data_;

    size_t vertex_data_capacity_ = 0;
    size_t index_data_capacity_ = 0;
    size_t indirect_command_data_capacity_ = 0;

    GLint uniform_texture_loc_;
    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;
    GLint uniform_model_loc_;

    GLenum rendering_mode_ = GL_TRIANGLES;

public:
    static const int CHUNK_COUNT_IN_HEIGHT = 16;
    static glm::ivec3 storage_sizes;

    glm::ivec3 rendering_center;
    int rendering_radius;
    unsigned int chunk_count = 0;

    // lists of planes in need of voxel face's merging
    static std::unordered_map<uint8_t, bool> X_planes_;
    static std::unordered_map<uint8_t, bool> Y_planes_;
    static std::unordered_map<uint8_t, bool> Z_planes_;

private:
    Chunks(const int radius, const glm::ivec3& center);
    Chunks(const Chunks&) = delete;
    ~Chunks();

public:
    void Draw(const Camera& camera) const;
};
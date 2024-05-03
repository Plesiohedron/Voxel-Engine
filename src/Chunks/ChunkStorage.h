#pragma once

#include "Chunk.h"

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/VAO.h"

#include "../Camera/Camera.h"

class ChunkStorage {
    friend class Engine;

private:
    mutable glm::mat4 model_;

    std::vector<Chunk*> chunks_;
    std::vector<GL::VAO*> chunks_VAOs_;

    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_;

    GLenum rendering_mode_ = GL_TRIANGLES;

    GLint uniform_texture_loc_;
    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;
    GLint uniform_model_loc_;

public:
    static const int CHUNKS_COUNT_IN_HEIGHT = 16;

    glm::ivec3 rendering_center;
    int rendering_radius;

    static glm::ivec3 storage_sizes;

    // lists of planes in need of voxel face's merging
    static std::unordered_map<uint8_t, bool> X_planes_;
    static std::unordered_map<uint8_t, bool> Y_planes_;
    static std::unordered_map<uint8_t, bool> Z_planes_;

private:
    ChunkStorage(const int radius, const glm::ivec3& center);
    ChunkStorage(const ChunkStorage&) = delete;
    ~ChunkStorage();

public:
    void Draw(const Camera& camera) const;
};
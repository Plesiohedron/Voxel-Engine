#pragma once

#include "Chunk.h"

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/ChunkVAO.h"
#include "../Camera/Camera.h"

class Chunks {
    friend class Engine;

private:
    glm::mat4* models_;
    Chunk** chunks_;
    GL::ChunkVAO** VAOs_;

    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_;

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

private:
    Chunks(const int radius, const glm::ivec3& center);
    Chunks(const Chunks&) = delete;
    ~Chunks();

public:
    void Draw(const Camera& camera) const;
};
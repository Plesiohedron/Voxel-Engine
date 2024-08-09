#pragma once

#include "Chunk.h"

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/SChunkVAO.h"
#include "../GL/SSBO.h"
#include "../Camera/Camera.h"

class Chunks {
    friend class Engine;

private:
    std::vector<glm::mat4> models_;
    Chunk** chunks_;

    GL::SChunkVAO** VAOs_;
    GLuint unified_EBO_lines;
    GLuint unified_EBO_triangles;
    GL::SSBO matrices_SSBO_;

    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_;

    GLint uniform_texture_loc_;
    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;
    GLint uniform_model_index_loc_;

    GLenum rendering_mode_ = GL_TRIANGLES;

public:
    static const int UNIFIED_EBO_TRIANGLES_SIZE = Chunk::INDEXES_COUNT_PER_SQUARE * Chunk::FACES_COUNT_PER_CUBE * Chunk::VOLUME / 2;
    static const int LINES_COUNT_PER_SQUARE = 10;
    static const int UNIFIED_EBO_LINES_SIZE = LINES_COUNT_PER_SQUARE * Chunk::FACES_COUNT_PER_CUBE * Chunk::VOLUME / 2;
    static const int CHUNK_COUNT_IN_HEIGHT = 16;
    static glm::ivec3 storage_sizes;

    glm::ivec3 rendering_center;
    int rendering_radius;
    unsigned int chunk_count = 0;

private:
    Chunks(const int radius, const glm::ivec3& center);
    Chunks(const Chunks&) = delete;
    ~Chunks();

    void UnbindVAO() const;

    Voxel* GetVoxel(int x, int y, int z) const;  // global coordinates
    inline Chunk* GetChunk(int x, int y, int z) const;  // local chunk coordinates
    void SetVoxel(int x, int y, int z, uint8_t id);  // global coordinates

    Voxel* RayCast(glm::vec3 a, glm::vec3 dir, float max_ray_length, glm::vec3& end, glm::vec3& normal, glm::vec3& iend);

public:
    void PollUpdates();
    void Draw(const Camera& camera) const;
};
#pragma once

#include <vector>
#include <stdint.h>

#include <GL/glew.h>
#include <GL/GL.h>

namespace GL {
    class ChunkVAO {
    public:
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;

        int indexes_count = 0;

    public:
        ChunkVAO();
        ChunkVAO(const ChunkVAO&) = delete;
        ~ChunkVAO();

        void Bind() const;
        void Draw(GLenum type) const;

        void InitializeVBO(const std::vector<uint64_t>& vertex_data);
        void InitializeEBO(const std::vector<uint32_t>& index_data);

        void PostInitialization() const;
    };
}  // namespace GL

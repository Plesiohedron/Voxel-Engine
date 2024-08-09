#pragma once

#include <vector>
#include <stdint.h>

#include <GL/glew.h>
#include <GL/GL.h>

namespace GL {
    class SChunkVAO {
    private:
        GLuint VAO = 0;

    public:
        GLuint VBO = 0;

    public:
        SChunkVAO();
        SChunkVAO(const SChunkVAO&) = delete;
        ~SChunkVAO();

        void Bind() const;

        void AllocateVBO(size_t vertex_data_size);
        void FillVBOSection(const uint64_t* vertex_data, int vertex_data_size, int offset);
    };
}  // namespace GL

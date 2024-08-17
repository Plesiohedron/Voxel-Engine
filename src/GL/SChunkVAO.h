#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include "../Chunks/Chunk.h"

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
        static void Unbind();

        void AllocateVBO(size_t vertex_data_size);
        void FillVBOSection(const Vertex* vertex_data, int vertex_data_size, int offset);
    };
}  // namespace GL

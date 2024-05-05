#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

namespace GL {
    struct IndirectCommand {
        GLuint count;
        GLuint instance_count;
        GLuint first_index;
        GLuint base_vertex;
        GLuint base_instance;
    };

    class SpecialVAO {
    public:
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;
        GLuint IB = 0;

    private:
        static const int CHUNK_VERTEX_SIZE_ = 3;
        static const int CHUNK_ATTRIBUTES_COUNT_ = 3;

    public:
        SpecialVAO();
        SpecialVAO(const SpecialVAO&) = delete;
        ~SpecialVAO();

        void Bind() const;

        void InitializeVBO(const GLushort* vertex_data, const size_t data_size);
        void InitializeEBO(const GLushort* index_data, const size_t data_size);
        void InitializeIB(const IndirectCommand* indirect_command_data, const size_t data_size);

        void PostInitialization() const;
    };
}  // namespace GL

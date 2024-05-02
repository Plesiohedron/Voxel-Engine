#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/glm.hpp>

namespace GL {
    class VAO {
    private:
        GLuint VAO_ = 0;
        std::vector<GLuint> VBOs_;
        GLuint EBO_ = 0;
        unsigned int indexes_count_ = 0;
        unsigned int attributes_count_ = 0;

        static const int CHUNK_VERTEX_SIZE_ = 3;
        static const int CHUNK_ATTRIBUTES_COUNT_ = 3;

    public:
        VAO();
        VAO(const VAO&) = delete;
        ~VAO();

        void Bind() const;
        void Draw(const unsigned int primitive_type) const;

        void InitializeBasicVBO(const std::vector<float>& vertices_data);
        void InitializeBasicVBO(const std::vector<glm::vec2>& vertices_data);
        void InitializeBasicVBO(const std::vector<glm::vec3>& vertices_data);
        void InitializeChunkVBO(const std::vector<GLushort>& vertices_data);
        void InitializeEBO(const std::vector<GLushort>& indexes_data);
        void PostInitialization() const;

        void DeinitializeVBO();
        void DeinitializeEBO();
    };
}  // namespace GL

#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/glm.hpp>

namespace GL {
class VAO {
private:
    GLuint* VBOs_;
    GLuint VAO_ = 0;
    GLuint EBO_ = 0;

    unsigned int VBOs_array_size_ = 0;
    unsigned int indexes_count_ = 0;
    unsigned int attributes_count_ = 0;

    static const int CHUNK_VERTEX_SIZE_ = 3;
    static const int CHUNK_ATTRIBUTES_COUNT_ = 3;

public:
    VAO();
    VAO(const VAO&) = delete;
    ~VAO();

    void Bind() const;
    void Draw(GLenum primitive_type) const;

    void InitializeBasicVBO(const std::vector<float>& vertex_data);
    void InitializeBasicVBO(const std::vector<glm::vec2>& vertex_data);
    void InitializeBasicVBO(const std::vector<glm::vec3>& vertex_data);

    void InitializeEBO(const std::vector<GLushort>& index_data);
    void InitializeEBO(const GLushort* index_data, unsigned int data_size);

    void PostInitialization() const;

    void DeinitializeVBO();
    void DeinitializeEBO();
};
} // namespace GL

#include "VAO.h"

GL::VAO::VAO() {
    glGenVertexArrays(1, &VAO_);
}

GL::VAO::~VAO() {
    glDeleteBuffers(VBOs_.size(), VBOs_.data());
    glDeleteBuffers(1, &EBO_);
    glDeleteVertexArrays(1, &VAO_);
}

void GL::VAO::Bind() const {
    glBindVertexArray(VAO_);
}

void GL::VAO::Draw(const GLenum primitive_type) const {
    assert(EBO_ != 0);

    glBindVertexArray(VAO_);

    for (int i = 0; i < attributes_count_; ++i) {
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glDrawElements(primitive_type, indexes_count_, GL_UNSIGNED_SHORT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (int i = 0; i < attributes_count_; ++i) {
        glDisableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

void GL::VAO::InitializeBasicVBO(const std::vector<float>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(float), vertices_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_.size(), 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_.push_back(VBO);
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec2>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(glm::vec2), vertices_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_.size(), 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_.push_back(VBO);
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec3>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(glm::vec3), vertices_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_.size(), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_.push_back(VBO);
}

void GL::VAO::InitializeChunkVBO(const std::vector<GLushort>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * CHUNK_VERTEX_SIZE_ * sizeof(GLushort), vertices_data.data(), GL_STATIC_DRAW);

    for (int i = 0; i < CHUNK_ATTRIBUTES_COUNT_; ++i) {
        glVertexAttribIPointer(i, 1, GL_UNSIGNED_SHORT, CHUNK_VERTEX_SIZE_ * sizeof(GLushort), reinterpret_cast<GLvoid*>(i * sizeof(GLushort)));
    }

    attributes_count_ = CHUNK_ATTRIBUTES_COUNT_;
    VBOs_.push_back(VBO);
}

void GL::VAO::InitializeEBO(const std::vector<GLushort>& indexes_data) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_data.size() * sizeof(GLushort), indexes_data.data(), GL_STATIC_DRAW);

    indexes_count_ = indexes_data.size();
}

void GL::VAO::PostInitialization() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GL::VAO::DeinitializeVBO() {
    for (auto& VBO : VBOs_) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glDeleteBuffers(1, &VBO);
    }

    attributes_count_ = 0;
    VBOs_.clear();
}

void GL::VAO::DeinitializeEBO() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glDeleteBuffers(1, &EBO_);
    EBO_ = 0;
}
#include "VAO.h"

GL::VAO::VAO() {
    glGenVertexArrays(1, &VAO_);
    VBOs_ = new GLuint[4];
}

void GL::VAO::Bind() const {
    glBindVertexArray(VAO_);
}

void GL::VAO::Draw(GLenum primitive_type) const {
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

void GL::VAO::InitializeBasicVBO(const std::vector<float>& vertex_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_array_size_, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec2>& vertex_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(glm::vec2), vertex_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_array_size_, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec3>& vertex_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(glm::vec3), vertex_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_array_size_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeEBO(const std::vector<GLushort>& index_data) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(GLushort), index_data.data(), GL_STATIC_DRAW);

    indexes_count_ = index_data.size();
}

void GL::VAO::InitializeEBO(const GLushort* index_data, unsigned int data_size) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size * sizeof(GLushort), index_data, GL_STATIC_DRAW);

    indexes_count_ = data_size;
}

void GL::VAO::PostInitialization() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GL::VAO::DeinitializeVBO() {
    glDeleteBuffers(VBOs_array_size_, VBOs_);

    attributes_count_ = 0;
    VBOs_array_size_ = 0;
    delete[] VBOs_;
}

void GL::VAO::DeinitializeEBO() {
    glDeleteBuffers(1, &EBO_);
    EBO_ = 0;
}

GL::VAO::~VAO() {
    glDeleteBuffers(VBOs_array_size_, VBOs_);
    glDeleteBuffers(1, &EBO_);
    glDeleteVertexArrays(1, &VAO_);

    delete[] VBOs_;
}
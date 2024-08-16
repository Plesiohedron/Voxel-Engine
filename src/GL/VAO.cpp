#include "VAO.h"

GL::VAO::VAO() {
    glGenVertexArrays(1, &VAO_);
    VBOs_.reserve(4);
}

void GL::VAO::Bind() const {
    glBindVertexArray(VAO_);
}

void GL::VAO::Unbind() {
    glBindVertexArray(0);
}

void GL::VAO::Draw(GLenum primitive_type) const {
    glBindVertexArray(VAO_);

    glDrawElements(primitive_type, indexes_count_, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
}

void GL::VAO::InitializeEBO(const std::vector<unsigned short int>& index_data) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(unsigned short int), index_data.data(), GL_STATIC_DRAW);

    indexes_count_ = index_data.size();
}

void GL::VAO::AllocateEBO(const std::vector<unsigned short int>& index_data) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.capacity() * sizeof(unsigned short int), nullptr, GL_DYNAMIC_DRAW);
}

void GL::VAO::AssignEBO(const std::vector<unsigned short int>& index_data) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_data.size() * sizeof(unsigned short int), index_data.data());

    indexes_count_ = index_data.size();
}

void GL::VAO::DeinitializeVBO() {
    glDeleteBuffers(VBOs_.size(), VBOs_.data());
    VBOs_.clear();
}

void GL::VAO::DeinitializeEBO() {
    glDeleteBuffers(1, &EBO_);
}

GL::VAO::~VAO() {
    glDeleteBuffers(VBOs_.size(), VBOs_.data());
    glDeleteBuffers(1, &EBO_);
    glDeleteVertexArrays(1, &VAO_);

    VBOs_.clear();
}
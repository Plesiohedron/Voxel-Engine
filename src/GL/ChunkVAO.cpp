#include "ChunkVAO.h"

#include <cassert>

GL::ChunkVAO::ChunkVAO() {
    glGenVertexArrays(1, &VAO);
}

void GL::ChunkVAO::Bind() const {
    glBindVertexArray(VAO);
}

void GL::ChunkVAO::Draw(GLenum type) const {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


    glDrawElements(type, indexes_count, GL_UNSIGNED_INT, nullptr);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
}

void GL::ChunkVAO::InitializeVBO(const std::vector<uint64_t>& vertex_data) {
    assert(VBO == 0);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(uint64_t), vertex_data.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, nullptr);
    //glVertexAttribIPointer()
}

void GL::ChunkVAO::InitializeEBO(const std::vector<uint32_t>& index_data) {
    assert(EBO == 0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data.size() * sizeof(uint32_t), index_data.data(), GL_STATIC_DRAW);

    indexes_count = index_data.size();
}

void GL::ChunkVAO::PostInitialization() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GL::ChunkVAO::~ChunkVAO() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
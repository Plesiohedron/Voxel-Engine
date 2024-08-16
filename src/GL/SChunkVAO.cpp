#include "SChunkVAO.h"

#include <cassert>

GL::SChunkVAO::SChunkVAO() {
    glGenVertexArrays(1, &VAO);
}

void GL::SChunkVAO::Bind() const {
    glBindVertexArray(VAO);
}

void GL::SChunkVAO::Unbind() {
    glBindVertexArray(0);
}

void GL::SChunkVAO::AllocateVBO(size_t vertex_data_size) {
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint64_t) * vertex_data_size, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GL::SChunkVAO::FillVBOSection(const uint64_t* vertex_data, int vertex_data_size, int offset) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(uint64_t) * offset, sizeof(uint64_t) * vertex_data_size, vertex_data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GL::SChunkVAO::~SChunkVAO() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}
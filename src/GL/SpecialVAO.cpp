#include "SpecialVAO.h"

#include <cassert>

GL::SpecialVAO::SpecialVAO() {
    glGenVertexArrays(1, &VAO);
}

void GL::SpecialVAO::Bind() const {
    glBindVertexArray(VAO);
}

void GL::SpecialVAO::InitializeVBO(const GLushort* vertex_data, const size_t data_size) {
    assert(VBO == 0);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data_size * sizeof(GLushort), vertex_data, GL_STATIC_DRAW);

    for (int i = 0; i < CHUNK_ATTRIBUTES_COUNT_; ++i) {
        glVertexAttribIPointer(i, 1, GL_UNSIGNED_SHORT, CHUNK_VERTEX_SIZE_ * sizeof(GLushort), reinterpret_cast<GLvoid*>(i * sizeof(GLushort)));
    }
}

void GL::SpecialVAO::InitializeEBO(const GLushort* index_data, const size_t data_size) {
    assert(EBO == 0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size * sizeof(GLushort), index_data, GL_STATIC_DRAW);
}

void GL::SpecialVAO::InitializeIB(const IndirectCommand* indirect_command_data, const size_t data_size) {
    assert(IB == 0);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IB);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, data_size * sizeof(IndirectCommand), indirect_command_data, GL_STATIC_DRAW);
}

void GL::SpecialVAO::PostInitialization() const {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GL::SpecialVAO::~SpecialVAO() {
    glDeleteBuffers(1, &IB);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
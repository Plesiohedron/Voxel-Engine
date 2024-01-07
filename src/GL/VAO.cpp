#include "VAO.h"

GL::VAO::VAO(Type type) {
    glGenVertexArrays(1, &mVAO);

    if (type == VAOchunk) {
        attributes = new int[3];
        attributes[0] = 1;
        attributes[1] = 2;
        attributes[2] = 3;

        attributes_count = 3;
        vertex_size = 6;
    } else if (type == Test) {
        attributes = new int[3];
        attributes[0] = 1;
        attributes[1] = 1;
        attributes[2] = 1;

        attributes_count = 3;
        vertex_size = 3;
    }
}

GL::VAO::~VAO() {
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);
    glDeleteVertexArrays(1, &mVAO);
}

void GL::VAO::bind() {
    glBindVertexArray(mVAO);
}

void GL::VAO::draw(unsigned primitiveType, unsigned indexes_count) {
    assert(mEBO != 0);

    glBindVertexArray(mVAO);

    for (int i = 0; i < attributes_count; i++) {
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glDrawElements(primitiveType, indexes_count, GL_UNSIGNED_SHORT, nullptr);

    for (int i = 0; i < attributes_count; i++) {
        glDisableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

void GL::VAO::initializeVBO_vertices(const float* vertices, unsigned vertices_count) {
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_count * vertex_size * sizeof(float), vertices, GL_STATIC_DRAW);
    
    unsigned offset = 0;
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (GLvoid*) (offset * sizeof(float)));

    offset += attributes[0];
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (GLvoid*) (offset * sizeof(float)));

    offset += attributes[1];
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (GLvoid*) (offset * sizeof(float)));
}

void GL::VAO::test(const GLushort* vertices, unsigned vertices_count) {
    glGenBuffers(1, &mVBO1);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO1);
    glBufferData(GL_ARRAY_BUFFER, vertices_count * vertex_size * sizeof(GLushort), vertices, GL_STATIC_DRAW);

    unsigned offset = 0;
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, vertex_size * sizeof(GLushort), (GLvoid*) (offset * sizeof(GLushort)));

    offset += attributes[0];
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_SHORT, vertex_size * sizeof(GLushort), (GLvoid*) (offset * sizeof(GLushort)));

    offset += attributes[1];
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, vertex_size * sizeof(GLushort), (GLvoid*) (offset * sizeof(GLushort)));
}

void GL::VAO::initializeEBO(const GLushort* indexes, unsigned indexes_count) {
    glGenBuffers(1, &mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_count * sizeof(GLushort), indexes, GL_STATIC_DRAW);
}

void GL::VAO::postInitialization() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GL::VAO::deinitializeVBO_vertices() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glDeleteBuffers(1, &mVBO);
    mVBO = 0;
}

void GL::VAO::deinitializeEBO() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glDeleteBuffers(1, &mEBO);
    mEBO = 0;
}
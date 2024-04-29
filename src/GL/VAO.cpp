#include "VAO.h"

GL::VAO::VAO(Type type) {
    glGenVertexArrays(1, &mVAO);

    if (type == VAOchunk) {
        attributes[0] = 1;
        attributes[1] = 1;
        attributes[2] = 1;

        attributes_count = 3;
        vertex_size = 3;
    } else if (type == VAOcrosshair) {
        attributes[0] = 1;
        attributes[1] = 1;
        attributes[2] = 0;

        attributes_count = 2;
        vertex_size = 2;
    } else if (type == Test) {
        attributes[0] = 1;
        attributes[1] = 1;
        attributes[2] = 1;

        attributes_count = 3;
        vertex_size = 3;
    }
}

GL::VAO::~VAO() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glDeleteBuffers(1, &mVBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glDeleteBuffers(1, &mEBO);

    glBindVertexArray(mVAO);
    glDeleteVertexArrays(1, &mVAO);
}

void GL::VAO::Bind() {
    glBindVertexArray(mVAO);
}

void GL::VAO::Draw(unsigned primitiveType) {
    assert(mEBO != 0);

    glBindVertexArray(mVAO);

    for (int i = 0; i < attributes_count; ++i) {
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glDrawElements(primitiveType, mIndexesCount, GL_UNSIGNED_SHORT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (int i = 0; i < attributes_count; ++i) {
        glDisableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

void GL::VAO::InitializeVBO(const std::vector<GLushort>& verticesData) {
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, verticesData.size() * vertex_size * sizeof(GLushort), verticesData.data(), GL_STATIC_DRAW);

    unsigned offset = 0;
    for (int i = 0; i < attributes_count; ++i) {
        glVertexAttribIPointer(i, 1, GL_UNSIGNED_SHORT, vertex_size * sizeof(GLushort), reinterpret_cast<GLvoid*>(offset * sizeof(GLushort)));
        offset += attributes[i];
    }
}

void GL::VAO::InitializeEBO(const std::vector<GLushort>& indexesData) {
    glGenBuffers(1, &mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexesData.size() * sizeof(GLushort), indexesData.data(), GL_STATIC_DRAW);

    mIndexesCount = indexesData.size();
}

void GL::VAO::PostInitialization() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GL::VAO::DeinitializeVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glDeleteBuffers(1, &mVBO);
    mVBO = 0;
}

void GL::VAO::DeinitializeEBO() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glDeleteBuffers(1, &mEBO);
    mEBO = 0;
}
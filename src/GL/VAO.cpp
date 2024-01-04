#include "VAO.h"

GL::VAO::VAO(Type objectType) {
    glGenVertexArrays(1, &mVAO);

    if (objectType == Chunk) {
        mVBO_size = CHUNK_VOLUME;
        mIBO_size = CHUNK_VOLUME;

        brithness.reserve(mVBO_size);
        UVs.reserve(mVBO_size);
        vertices.reserve(mVBO_size);
        //colors.reserve(mVBO_size);
        indexes.reserve(mVBO_size);
    } else if (objectType == Test) {
        brithness.reserve(4);
        UVs.reserve(4);
        vertices.reserve(4);
        colors.reserve(4);
        indexes.reserve(6);
    }
}

GL::VAO::~VAO() {
    for (int i = 0; i < mVBO_COUNT; i++) {
        if (mVBOs[i] != 0)
            glDeleteBuffers(1, (mVBOs + i));
    }
    glDeleteBuffers(1, &mIBO);
    glDeleteVertexArrays(1, &mVAO);
}

void GL::VAO::bind() {
    glBindVertexArray(mVAO);
}

void GL::VAO::draw(unsigned type) {
    assert(mIBO != 0);

    glBindVertexArray(mVAO);

    for (int i = 0; i < mVBO_COUNT; i++) {
        if (mVBOs[i] != 0)
            glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glDrawElements(type, indexes.size(), GL_UNSIGNED_INT, nullptr);

    for (int i = 0; i < mVBO_COUNT; i++) {
        if (mVBOs[i] != 0)
            glDisableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

void GL::VAO::initializeVBO_brithness() {
    glGenBuffers(1, mVBOs);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, brithness.size() * sizeof(float), brithness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GL::VAO::initializeVBO_UVs() {
    glGenBuffers(1, (mVBOs + 1));
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), UVs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GL::VAO::initializeVBO_vertices() {
    glGenBuffers(1, (mVBOs + 2));
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GL::VAO::initializeVBO_colors() {
    glGenBuffers(1, (mVBOs + 3));
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GL::VAO::initialize_IBO() {
    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned), indexes.data(), GL_STATIC_DRAW);
}

void GL::VAO::postInitialization() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GL::VAO::deinitializeVBO_brithness() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[0]);
    glDeleteBuffers(1, mVBOs);
    mVBOs[0] = 0;
}

void GL::VAO::deinitializeVBO_UVs() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[1]);
    glDeleteBuffers(1, (mVBOs + 1));
    mVBOs[1] = 0;
}

void GL::VAO::deinitializeVBO_vertices() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[2]);
    glDeleteBuffers(1, (mVBOs + 2));
    mVBOs[2] = 0;
}

void GL::VAO::deinitializeVBO_colors() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBOs[3]);
    glDeleteBuffers(1, (mVBOs + 3));
    mVBOs[3] = 0;
}

void GL::VAO::deinitialize_IBO() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glDeleteBuffers(1, &mIBO);
    mIBO = 0;
}
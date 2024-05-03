#include "VAO.h"

GL::VAO::VAO() {
    glGenVertexArrays(1, &VAO_);
    VBOs_ = new GLuint[4];
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
    glVertexAttribPointer(VBOs_array_size_, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec2>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(glm::vec2), vertices_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_array_size_, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeBasicVBO(const std::vector<glm::vec3>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * sizeof(glm::vec3), vertices_data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(VBOs_array_size_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    ++attributes_count_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeChunkVBO(const std::vector<unsigned short int>& vertices_data) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_data.size() * CHUNK_VERTEX_SIZE_ * sizeof(unsigned short int), vertices_data.data(), GL_STATIC_DRAW);

    for (int i = 0; i < CHUNK_ATTRIBUTES_COUNT_; ++i) {
        glVertexAttribIPointer(i, 1, GL_UNSIGNED_SHORT, CHUNK_VERTEX_SIZE_ * sizeof(unsigned short int), reinterpret_cast<GLvoid*>(i * sizeof(unsigned short int)));
    }

    attributes_count_ = CHUNK_ATTRIBUTES_COUNT_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeChunkVBO(const unsigned short int* vertices_data, const unsigned int data_size) {
    GLuint VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data_size * CHUNK_VERTEX_SIZE_ * sizeof(unsigned short int), vertices_data, GL_STATIC_DRAW);

    for (int i = 0; i < CHUNK_ATTRIBUTES_COUNT_; ++i) {
        glVertexAttribIPointer(i, 1, GL_UNSIGNED_SHORT, CHUNK_VERTEX_SIZE_ * sizeof(unsigned short int), reinterpret_cast<GLvoid*>(i * sizeof(unsigned short int)));
    }

    attributes_count_ = CHUNK_ATTRIBUTES_COUNT_;
    VBOs_[VBOs_array_size_] = VBO;
    ++VBOs_array_size_;
}

void GL::VAO::InitializeEBO(const std::vector<unsigned short int>& indexes_data) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_data.size() * sizeof(unsigned short int), indexes_data.data(), GL_STATIC_DRAW);

    indexes_count_ = indexes_data.size();
}

void GL::VAO::InitializeEBO(const unsigned short int* indexes_data, const unsigned int data_size) {
    assert(EBO_ == 0);

    glGenBuffers(1, &EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size * sizeof(unsigned short int), indexes_data, GL_STATIC_DRAW);

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
#include "SSBO.h"

#include <glm/gtc/type_ptr.hpp>

GL::SSBO::SSBO() {
    glGenBuffers(1, &SSBO_);
}

GL::SSBO::~SSBO() {
    glDeleteBuffers(1, &SSBO_);
}

void GL::SSBO::InitializeMatrices(const glm::mat4* matrices_data, int data_size, int binding_point) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data_size * sizeof(glm::mat4), matrices_data, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, SSBO_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
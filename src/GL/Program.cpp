#include "Program.h"

#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

const int INFO_LOG_LENGTH = 512;


GL::Program::Program(const std::string& name) {
    mProgram = glCreateProgram();

    mVertexShader = LoadShader(("res/glsl/" + name + ".vert").c_str(), GL_VERTEX_SHADER);
    mFragmentShader = LoadShader(("res/glsl/" + name + ".frag").c_str(), GL_FRAGMENT_SHADER);
}

GL::Program::~Program() {
    glDetachShader(mProgram, mVertexShader);
    glDetachShader(mProgram, mFragmentShader);

    glDeleteShader(mVertexShader);
    glDeleteShader(mFragmentShader);

    glDeleteProgram(mProgram);
}

void GL::Program::link() {
    glAttachShader(mProgram, mVertexShader);
    glAttachShader(mProgram, mFragmentShader);
    glLinkProgram(mProgram);

    GLint status;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    if (!status) {
        char buf[INFO_LOG_LENGTH];
        glGetShaderInfoLog(mProgram, INFO_LOG_LENGTH, nullptr, buf);
        std::cout << buf << std::endl;

        throw std::runtime_error("Failed to link shader!");
    }
}

void GL::Program::bindAttribute(GLuint index, const char* name) {
    glBindAttribLocation(mProgram, index, name);
}

GLint GL::Program::bindUniform(const char* name) {
    return glGetUniformLocation(mProgram, name);
}

GLint GL::Program::getUniformLocation(const char* name) {
    return glGetUniformLocation(mProgram, name);
}

void GL::Program::use() {
    glUseProgram(mProgram);
}

void GL::Program::uniformMatrix(GLint uniform, glm::mat4 matrix) {
    glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(matrix));
}

GLuint GL::Program::LoadShader(const char* path, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);

    std::ifstream fin(path);
    std::string shaderCode = {std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};

    const char* code = shaderCode.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buf[INFO_LOG_LENGTH];
        glGetShaderInfoLog(shader, INFO_LOG_LENGTH, nullptr, buf);
        std::cout << path << ":" << std::endl << buf << std::endl;

        throw std::runtime_error("Failed to compile shader!");
    }

    return shader;
}
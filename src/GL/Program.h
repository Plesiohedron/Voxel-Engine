#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/glm.hpp>
#include <string>

namespace GL {
    class Program {
    private:
        GLuint mProgram;
        GLuint mVertexShader;
        GLuint mFragmentShader;

        GLuint LoadShader(const char* path, GLenum shaderType);

    public:
        Program(const std::string& name);
        ~Program();

        void link();
        void bindAttribute(GLuint index, const char* name);
        GLint bindUniform(const char* name);
        GLint getUniformLocation(const char* name);
        void use();
        void uniformMatrix(GLint uniform, glm::mat4 matrix);
        void uniformMatrix(const char* name, glm::mat4 matrix);
    };
}

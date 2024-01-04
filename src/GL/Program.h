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

        void Link();
        void BindAttribute(GLuint index, const char* name);
        GLint BindUniform(const char* name);
        void Use();
        void BrithnessAnimation(const char* name, float value);
        void UniformMatrix(const char* name, glm::mat4 matrix);
    };
}

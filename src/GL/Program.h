#pragma once

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Exceptions/Exceptions.h"

namespace GL {
    class Program {
    public:
        Program(const std::string& name);
        ~Program();

        void Link() const;
        void Use() const;
        static void Unuse();

        void BindAttribute(GLuint index, const char* name) const;
        GLint GetUniformLocation(const char* name) const;
        void UniformInt(GLint location, int value) const;
        void UniformMatrix(GLint uniform, const glm::mat4& matrix) const;
        void UniformTexture(GLint location, GLint number) const;

    private:
        GLuint LoadShader(const char* path, const GLenum shader_type) const;

    private:
        GLuint program_;
        GLuint vertex_shader_;
        GLuint fragment_shader_;

        static const int INFO_LOG_LENGTH_ = 512;
    };
}  // namespace GL

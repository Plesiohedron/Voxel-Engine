#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>

#include "../Load/Image.h"

namespace GL {
    class Texture2D {
    private:
        GLuint handle_;

    public:
        GLenum GL_format;

        unsigned int width;
        unsigned int height;

    public:
        Texture2D();
        Texture2D(const Texture2D&) = delete;
        ~Texture2D();

        void Bind() const;
        void SetImage(const Image& image);
        void SetEmpty(const unsigned int texture_width, const unsigned int texture_height);
    };
}  // namespace GL
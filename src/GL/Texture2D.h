#pragma once

#include <cassert>
#include <GL/glew.h>
#include <GL/GL.h>

#include "../Load/Image.h"

namespace GL {
    class Texture2D {
    private:
        GLuint mHandle;

    public:
        GLenum glFormat;
        unsigned int width, height;

        Texture2D();
        Texture2D(const Texture2D&) = delete;
        ~Texture2D();

        void bind();
        void setImage(const Image& image);
        void set();
    };
}
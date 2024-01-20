#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include "../Load/Image.h"

#include <cassert>

namespace GL {
    class Texture3D {
    private:
        GLuint mHandle;

    public:
        GLenum glFormat_CPU;
        GLenum glFormat_GPU;
        unsigned int width, height;

        Texture3D();
        Texture3D(const Texture3D&) = delete;
        ~Texture3D();

        void bind();
        void setImage(const Image& image);
    };
}
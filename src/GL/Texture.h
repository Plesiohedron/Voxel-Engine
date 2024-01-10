#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include "../Load/Image.h"

namespace GL {
    class Texture {
    private:
        GLuint mHandle;

    public:
        GLenum glFormat;
        unsigned int width, height;

        Texture();
        Texture(const Texture&) = delete;
        ~Texture();

        void bind();
        void setImage(const Image& image);
    };
}
#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include "../Load/Image.h"

#include <cassert>

namespace GL {
    class Texture3D {
    private:
        GLuint handle_;

        static const int LAYERS_COUNT_ = 256;
        static const int VOXEL_IMAGE_WIDTH_ = 32;
        static const int VOXEL_IMAGE_HEIGHT_ = 32;
        static const int CHANNELS_COUNT_ = 4;

    public:
        GLenum GL_format_CPU;
        GLenum GL_format_GPU;

        unsigned int width;
        unsigned int height;

    public:
        Texture3D();
        Texture3D(const Texture3D&) = delete;
        ~Texture3D();

        void Bind() const;
        void SetAtlas(const Image& image);

    private:
        void MakeSubImages(const std::vector<char>& image_data) const;
    };
}
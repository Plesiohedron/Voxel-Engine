#include "Texture3D.h"

GL::Texture3D::Texture3D() {
    glGenTextures(1, &handle_);
}

GL::Texture3D::~Texture3D() {
    glDeleteTextures(1, &handle_);
}

void GL::Texture3D::Bind() const {
    glBindTexture(GL_TEXTURE_2D_ARRAY, handle_);
}

void GL::Texture3D::SetAtlas(const Image& image) {
    switch (image.format) {
        case Image::RGB:
            GL_format_CPU = GL_RGB;
            GL_format_GPU = GL_RGB8;
            break;

        case Image::RGBA:
            GL_format_CPU = GL_RGBA;
            GL_format_GPU = GL_RGBA8;
            break;
            
        default:
            assert(0);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, handle_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_format_GPU, VOXEL_IMAGE_WIDTH_, VOXEL_IMAGE_HEIGHT_, LAYERS_COUNT_, 0, GL_format_CPU, GL_UNSIGNED_BYTE, nullptr);

    MakeSubImages(image.data);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GL::Texture3D::MakeSubImages(const std::vector<char>& image_data) const {
    GLubyte sub_image[4 * 32 * 32]{0};

    int i;
    for (int layer = 0; layer < LAYERS_COUNT_; ++layer) {
        i = 0;

        for (int y = 0; y < VOXEL_IMAGE_HEIGHT_; ++y) {
            for (int x = 0; x < VOXEL_IMAGE_WIDTH_; ++x) {
                for (int c = 0; c < CHANNELS_COUNT_; ++c) {
                    sub_image[i] = image_data[4 * 32 * 512 * (layer / 16) + 4 * 32 * (layer % 16) + 4 * (x + 512 * y) + c];
                    ++i;
                }
            }
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, VOXEL_IMAGE_WIDTH_, VOXEL_IMAGE_HEIGHT_, 1, GL_format_CPU, GL_UNSIGNED_BYTE, sub_image);
    }
}

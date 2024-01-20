#include "Texture3D.h"

GL::Texture3D::Texture3D() {
    glGenTextures(1, &mHandle);
}

GL::Texture3D::~Texture3D() {
    glDeleteTextures(1, &mHandle);
}

void GL::Texture3D::bind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, mHandle);
}

void GL::Texture3D::setImage(const Image& image) {
    switch (image.getFormat()) {
        case Image::RGB:
            glFormat_CPU = GL_RGB;
            glFormat_GPU = GL_RGB8;
            break;

        case Image::RGBA:
            glFormat_CPU = GL_RGBA;
            glFormat_GPU = GL_RGBA8;
            break;
            
        default:
            assert(0);
    }

    width = 32;
    height = 32;
    const int numLayers = 256;
    const std::vector<char> ImageData = image.getData();
    GLubyte SubImage[4 * 32 * 32] = {};

    glBindTexture(GL_TEXTURE_2D_ARRAY, mHandle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, glFormat_GPU, width, height, numLayers, 0, glFormat_CPU, GL_UNSIGNED_BYTE, nullptr);

    for (int layer = 0; layer < numLayers; ++layer) {
        int i = 0;

        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                for (int c = 0; c < 4; ++c) {
                    SubImage[i] = ImageData[4 * 32 * 512 * (layer / 16) + 4 * 32 * (layer % 16) + 4 * (x + 512 * y) + c];
                    ++i;
                }
            }
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, glFormat_CPU, GL_UNSIGNED_BYTE, SubImage);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

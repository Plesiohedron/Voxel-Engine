#include "Texture.h"

#include <cassert>

GL::Texture::Texture() {
    glGenTextures(1, &mHandle);
}

GL::Texture::~Texture() {
    glDeleteTextures(1, &mHandle);
}

void GL::Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, mHandle);
}

void GL::Texture::setImage(const Image& image) {
    glBindTexture(GL_TEXTURE_2D, mHandle);

    switch (image.getFormat()) {
        case Image::RGB:
            glFormat = GL_RGB;
            break;

        case Image::RGBA:
            glFormat = GL_RGBA;
            break;

        default:
            assert(0);
    }

    width = image.getWidth();
    height = image.getHeight();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, image.getData().data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void GL::Texture::MakeWindowRegionTexture() {
    glBindTexture(GL_TEXTURE_2D, mHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

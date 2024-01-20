#include "Texture2D.h"
#include <iostream>

GL::Texture2D::Texture2D() {
    glGenTextures(1, &mHandle);
}

GL::Texture2D::~Texture2D() {
    glDeleteTextures(1, &mHandle);
}

void GL::Texture2D::bind() {
    glBindTexture(GL_TEXTURE_2D, mHandle);
}

void GL::Texture2D::setImage(const Image& image) {
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


    glBindTexture(GL_TEXTURE_2D, mHandle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, image.getData().data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glBindTexture: " << error << std::endl;
    }
}

void GL::Texture2D::set() {
    glBindTexture(GL_TEXTURE_2D, mHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

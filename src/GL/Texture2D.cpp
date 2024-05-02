#include "Texture2D.h"

GL::Texture2D::Texture2D() {
    glGenTextures(1, &handle_);
}

GL::Texture2D::~Texture2D() {
    glDeleteTextures(1, &handle_);
}

void GL::Texture2D::Bind() const {
    glBindTexture(GL_TEXTURE_2D, handle_);
}

void GL::Texture2D::SetImage(const Image& image) {
    switch (image.format) {
        case Image::RGB:
            GL_format = GL_RGB;
            break;

        case Image::RGBA:
            GL_format = GL_RGBA;
            break;
    }

    width = image.width;
    height = image.height;


    glBindTexture(GL_TEXTURE_2D, handle_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_format, width, height, 0, GL_format, GL_UNSIGNED_BYTE, image.data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void GL::Texture2D::SetEmpty(const unsigned int texture_width, const unsigned int texture_height) {
    glBindTexture(GL_TEXTURE_2D, handle_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

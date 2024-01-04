#include "Image.h"

#include <stdexcept>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <../../include/stb_image.h>

Image::Image(unsigned width, unsigned height, const std::vector<char>& data, Format format)
    : mWidth(width), mHeight(height), mData(data), mFormat(format) { }

Image Image::LoadImage(const std::string& path) {
    int x, y, channels;

    std::unique_ptr<stbi_uc> data = std::unique_ptr<stbi_uc>(stbi_load(("res/texture/" + path).c_str(), &x, &y, &channels, 0));

    if (data == nullptr)
        throw std::runtime_error(path + '\n' + "Failed to initialize image!");

    Format format;

    switch (channels) {
        case 3:
            format = RGB;
            break;

        case 4:
            format = RGBA;
            break;

        default:
            throw std::runtime_error("Incorrect number of channels (" + std::to_string(channels) + ") in file: " + path);
    }

    int size = x * y * channels;
    return Image(x, y, std::vector<char> {data.get(), data.get() + size}, format);
}

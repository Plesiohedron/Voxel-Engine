#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <../../include/stb_image.h>

Image::Image(const unsigned width, const unsigned height, const std::vector<char>& data, const Format format)
    : width(width), height(height), data(data), format(format) { }

Image Image::LoadImage(const std::string& path) {
    int x, y, channels;

    std::unique_ptr<stbi_uc> data = std::unique_ptr<stbi_uc>(stbi_load(("res/texture/" + path).c_str(), &x, &y, &channels, 0));

    if (data == nullptr) {
        throw STBImageError(path + '\n' + "Failed to initialize image!");
    }

    Format format;
    switch (channels) {
        case 3:
            format = RGB;
            break;

        case 4:
            format = RGBA;
            break;

        default:
            throw STBImageError("Incorrect number of channels (" + std::to_string(channels) + ") in file: " + path);
    }

    int size = x * y * channels;
    return Image(x, y, std::vector<char>{data.get(), data.get() + size}, format);
}

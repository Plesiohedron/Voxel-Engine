#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../Exceptions/Exceptions.h"

class Image {
public:
    enum Format {RGB, RGBA};

public:
    Format format;
    unsigned int width;
    unsigned int height;
    std::vector<char> data;

public:
    Image(const unsigned width, const unsigned height, const std::vector<char>& data, const Format format);
    static Image LoadImage(const std::string& path);
};

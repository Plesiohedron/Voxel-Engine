#pragma once

#include <vector>
#include <string>

class Image {
public:
    enum Format {RGB, RGBA};

private:
    Format mFormat;
    unsigned mWidth, mHeight;
    std::vector<char> mData;

public:
    Image(unsigned width, unsigned height, const std::vector<char>& data, Format format);

    Format getFormat() const {
        return mFormat;
    }

    unsigned getWidth() const {
        return mWidth;
    }

    unsigned getHeight() const {
        return mHeight;
    }

    const std::vector<char>& getData() const {
        return mData;
    }

    static Image LoadImage(const std::string& path);
};

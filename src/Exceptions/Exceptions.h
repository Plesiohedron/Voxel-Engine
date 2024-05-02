#pragma once

#include <stdexcept>

class GLFWError : public std::runtime_error {
public:
    inline explicit GLFWError(const std::string message) : std::runtime_error(message){};
};

class OpenGLError : public std::runtime_error {
public:
    inline explicit OpenGLError(const std::string message) : std::runtime_error(message){};
};

class STBImageError : public std::runtime_error {
public:
    inline explicit STBImageError(const std::string message) : std::runtime_error(message){};
};
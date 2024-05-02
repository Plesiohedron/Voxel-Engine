#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Exceptions/Exceptions.h"

class Window {
    friend class Engine;

public:
    bool IsShouldClose() const;
    void SetShouldClose(const bool flag) const;
    void SwapBuffers() const;
    float GetAspect() const;

public:
    int width;
    int height;

    bool is_iconfied{false};
    bool is_resized{false};

    GLFWwindow* window;

private:
    Window(const int window_width, const int window_height, const char* window_title);
    ~Window();
};

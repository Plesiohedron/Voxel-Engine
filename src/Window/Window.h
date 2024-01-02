#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

struct Events;

class Window {
    static GLFWwindow* mWindow;

    static int mWidth;
    static int mHeight;

    static bool mIconfied;
    static bool mResized;

public:
    friend struct Events;

    static void initialize(int width, int height, const char* title);
    static void deinitialize();

    static bool isShouldClose();
    static void setShouldClose(bool flag);
    static void swapBuffers();
};

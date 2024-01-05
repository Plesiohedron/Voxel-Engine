#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

class Window {
    static GLFWwindow* mWindow;

    static int mWidth;
    static int mHeight;

public:
    friend class Events;
    friend class Engine;

    static bool mIconfied;
    static bool mResized;

    static void initialize(int width, int height, const char* title);
    static void deinitialize();

    static bool isShouldClose();
    static void setShouldClose(bool flag);
    static void swapBuffers();

    static float getAspect();
};

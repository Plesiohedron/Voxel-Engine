#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

class Window {
friend class Events;
friend class Engine;

private:
    static GLFWwindow* mWindow;

    static int mWidth;
    static int mHeight;
public:
    static bool isIconfied;
    static bool isResized;

    static void Initialize(int windowWidth, int windowHeight, const char* windowTitle);
    static void Deinitialize();

    static bool IsShouldClose();
    static void SetShouldClose(bool flag);
    static void SwapBuffers();

    static float GetAspect();
};

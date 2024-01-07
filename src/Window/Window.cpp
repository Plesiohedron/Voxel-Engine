#include "Window.h"

GLFWwindow* Window::mWindow;

int Window::mWidth;
int Window::mHeight;

bool Window::mIconfied;
bool Window::mResized;

void Window::initialize(int width, int height, const char* title) {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!mWindow)
        throw std::runtime_error("Failed to create window!");

    mWidth = width;
    mHeight = height;

    glfwMakeContextCurrent(mWindow);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize GLEW!");

    glViewport(0, 0, mWidth, mHeight);
}

bool Window::isShouldClose() {
    return glfwWindowShouldClose(mWindow);
}

void Window::setShouldClose(bool flag) {
    glfwSetWindowShouldClose(mWindow, flag);
}

void Window::swapBuffers() {
    glfwSwapBuffers(mWindow);
}

float Window::getAspect() {
    return static_cast<float>(mWidth) / mHeight;
}

void Window::deinitialize() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

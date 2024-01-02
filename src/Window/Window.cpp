#include "Window.h"

GLFWwindow* Window::mWindow;

int Window::mWidth;
int Window::mHeight;

bool Window::mIconfied;
bool Window::mResized;

void Window::initialize(int width, int height, const char* title) {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    Window::mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!mWindow)
        throw std::runtime_error("Failed to create window!");

    Window::mWidth = width;
    Window::mHeight = height;

    glfwMakeContextCurrent(mWindow);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize GLEW!");

    glViewport(0, 0, Window::mWidth, Window::mHeight);
}

bool Window::isShouldClose() {
    return glfwWindowShouldClose(Window::mWindow);
}

void Window::setShouldClose(bool flag) {
    glfwSetWindowShouldClose(Window::mWindow, flag);
}

void Window::swapBuffers() {
    glfwSwapBuffers(Window::mWindow);
}

void Window::deinitialize() {
    glfwDestroyWindow(Window::mWindow);
    glfwTerminate();
}

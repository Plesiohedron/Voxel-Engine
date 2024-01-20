#include "Window.h"

GLFWwindow* Window::mWindow;

int Window::mWidth;
int Window::mHeight;

bool Window::isIconfied;
bool Window::isResized;

void Window::Initialize(int windowWidth, int windowHeight, const char* windowTitle) {
    mWidth = windowWidth;
    mHeight = windowHeight;

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    mWindow = glfwCreateWindow(mWidth, mHeight, windowTitle, nullptr, nullptr);
    if (!mWindow) {
        throw std::runtime_error("Failed to create window!");
    }

    glfwMakeContextCurrent(mWindow);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW!");
    }

    glViewport(0, 0, mWidth, mHeight);
}

bool Window::IsShouldClose() {
    return glfwWindowShouldClose(mWindow);
}

void Window::SetShouldClose(bool flag) {
    glfwSetWindowShouldClose(mWindow, flag);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(mWindow);
}

float Window::GetAspect() {
    return static_cast<float>(mWidth) / mHeight;
}

void Window::Deinitialize() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

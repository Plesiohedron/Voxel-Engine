#include "Window.h"

#include "../Events/Events.h"

Window::Window(const int window_width, const int window_height, const char* window_title) 
    : width(window_width), height(window_height) {
    if (!glfwInit()) {
        throw GLFWError("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);
    if (!window) {
        throw GLFWError("Failed to create window!");
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw OpenGLError("Failed to initialize GLEW!");
    }

    glViewport(0, 0, width, height);

    Events::Initialize(this);
}

bool Window::IsShouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::SetShouldClose(const bool flag) const {
    glfwSetWindowShouldClose(window, flag);
}

void Window::SwapBuffers() const {
    glfwSwapBuffers(window);
}

float Window::GetAspect() const {
    return static_cast<float>(width) / height;
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

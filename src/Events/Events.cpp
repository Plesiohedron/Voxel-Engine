#include "Events.h"

#include <iostream>

bool Events::keys[1032] = {false};

float Events::cursor_x = 0.0f;
float Events::cursor_y = 0.0f;

float Events::cursor_delta_x = 0.0f;
float Events::cursor_delta_y = 0.0f;

bool Events::cursor_is_moving = false;
bool Events::cursor_is_locked = false;

unsigned long long int Events::current_frame = 0;
unsigned long long int Events::frames[1032] = {0};

void Events::Initialize() {
    glfwSetWindowSizeCallback(Window::mWindow, WindowResizeCallback);
    glfwSetKeyCallback(Window::mWindow, KeyCallback);
    glfwSetMouseButtonCallback(Window::mWindow, MouseCallback);
    glfwSetCursorPosCallback(Window::mWindow, CursorPosCallback);
    glfwSetWindowIconifyCallback(Window::mWindow, WindowIconifyCallback);
}

void Events::PollEvents() {
    cursor_delta_x = 0;
    cursor_delta_y = 0;
    ++current_frame;
    glfwPollEvents();
}

bool Events::KeyIsClicked(int key) {
    if (0 <= key && key < MOUSE_BUTTONS) {
        return keys[key] && (frames[key] == current_frame);
    }

    return false;
}

bool Events::KeyIsPressed(int key) {
    if (0 <= key && key < MOUSE_BUTTONS) {
        return keys[key];
    }

    return false;
}

bool Events::MouseIsClicked(int button) {
    return keys[MOUSE_BUTTONS + button] && (frames[MOUSE_BUTTONS + button] == current_frame);
}

bool Events::MouseIsPressed(int button) {
    return keys[MOUSE_BUTTONS + button];
}

void Events::SwitchCursor() {
    cursor_is_locked = !cursor_is_locked;
    glfwSetInputMode(Window::mWindow, GLFW_CURSOR, cursor_is_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::WindowResizeCallback(GLFWwindow* window, int width, int height) {
    if (width % 2 == 1)
        width--;
    if (height % 2 == 1)
        height--;

    glViewport(0, 0, width, height);

    Window::mWidth = width;
    Window::mHeight = height;

    Window::isResized = true;
}

void Events::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        keys[key] = true;
        frames[key] = current_frame;
    } else if (action == GLFW_RELEASE) {
        keys[key] = false;
        frames[key] = current_frame;
    }
}

void Events::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        keys[MOUSE_BUTTONS + button] = true;
        frames[MOUSE_BUTTONS + button] = current_frame;
    } else if (action == GLFW_RELEASE) {
        keys[MOUSE_BUTTONS + button] = false;
        frames[MOUSE_BUTTONS + button] = current_frame;
    }
}

void Events::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    float float_xpos = static_cast<float>(xpos);
    float float_ypos = static_cast<float>(ypos);

    if (cursor_is_moving) {
        cursor_delta_x += float_xpos - cursor_x;
        cursor_delta_y += float_ypos - cursor_y;
    } else {
        cursor_is_moving = true;
    }

    cursor_x = float_xpos;
    cursor_y = float_ypos;
}

void Events::WindowIconifyCallback(GLFWwindow* window, int iconified) {
    Window::isIconfied = iconified;
}

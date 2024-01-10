#include "Events.h"

#include <iostream>

bool Events::_keys[1032] = {false};

float Events::_cursor_x = 0.0f;
float Events::_cursor_y = 0.0f;

float Events::_cursor_delta_x = 0.0f;
float Events::_cursor_delta_y = 0.0f;

bool Events::_cursor_isMoving = false;
bool Events::_cursor_isLocked = false;

unsigned long long int Events::_current_frame = 0;
unsigned long long int Events::_frames[1032] = {0};

void Events::initialize() {
    glfwSetWindowSizeCallback(Window::mWindow, WindowResize_Callback);
    glfwSetKeyCallback(Window::mWindow, Key_Callback);
    glfwSetMouseButtonCallback(Window::mWindow, Mouse_Callback);
    glfwSetCursorPosCallback(Window::mWindow, CursorPos_Callback);
    glfwSetWindowIconifyCallback(Window::mWindow, WindowIconify_Callback);
}

void Events::pollEvents() {
    _cursor_delta_x = 0;
    _cursor_delta_y = 0;
    _current_frame++;
    glfwPollEvents();
}

bool Events::key_isClicked(int key) {
    if (0 <= key && key < MOUSE_BUTTONS)
        return _keys[key] && (_frames[key] == _current_frame);

    return false;
}

bool Events::key_isPressed(int key) {
    if (0 <= key && key < MOUSE_BUTTONS)
        return _keys[key];

    return false;
}

bool Events::mouse_isClicked(int button) {
    return _keys[MOUSE_BUTTONS + button] && (_frames[MOUSE_BUTTONS + button] == _current_frame);
}

bool Events::mouse_isPressed(int button) {
    return _keys[MOUSE_BUTTONS + button];
}

void Events::switchCursor() {
    _cursor_isLocked = !_cursor_isLocked;
    glfwSetInputMode(Window::mWindow, GLFW_CURSOR, _cursor_isLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::WindowResize_Callback(GLFWwindow* window, int width, int height) {
    if (width % 2 == 1)
        width--;
    if (height % 2 == 1)
        height--;

    glViewport(0, 0, width, height);

    Window::mWidth = width;
    Window::mHeight = height;

    Window::isResized = true;
}

void Events::Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        _keys[key] = true;
        _frames[key] = _current_frame;
    } else if (action == GLFW_RELEASE) {
        _keys[key] = false;
        _frames[key] = _current_frame;
    }
}

void Events::Mouse_Callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        _keys[MOUSE_BUTTONS + button] = true;
        _frames[MOUSE_BUTTONS + button] = _current_frame;
    } else if (action == GLFW_RELEASE) {
        _keys[MOUSE_BUTTONS + button] = false;
        _frames[MOUSE_BUTTONS + button] = _current_frame;
    }
}

void Events::CursorPos_Callback(GLFWwindow* window, double xpos, double ypos) {

    float f_xpos = static_cast<float>(xpos);
    float f_ypos = static_cast<float>(ypos);

    if (_cursor_isMoving) {
        _cursor_delta_x += f_xpos - _cursor_x;
        _cursor_delta_y += f_ypos - _cursor_y;
    } else
        _cursor_isMoving = true;

    _cursor_x = f_xpos;
    _cursor_y = f_ypos;
}

void Events::WindowIconify_Callback(GLFWwindow* window, int iconified) {
    if (iconified)
        Window::isIconfied = true;
    else
        Window::isIconfied = false;
}

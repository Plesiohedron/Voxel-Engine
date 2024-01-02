#pragma once

#include "..\Window\Window.h"

struct Events {
    static const short int MOUSE_BUTTONS = 1024;

    static bool _keys[1032];

    static float _cursor_x;
    static float _cursor_y;

    static float _cursor_delta_x;
    static float _cursor_delta_y;

    static bool _cursor_isMoving;
    static bool _cursor_isLocked;

    static unsigned long long int _current_frame;
    static unsigned long long int _frames[1032];

public:
    static void initialize();
    static void pollEvents();

    static bool key_isClicked(int key);
    static bool key_isPressed(int key);

    static bool mouse_isClicked(int button);
    static bool mouse_isPressed(int button);

private:
    static void WindowResize_Callback(GLFWwindow* window, int width, int height);
    static void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void Mouse_Callback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPos_Callback(GLFWwindow* window, double xpos, double ypos);
    static void WindowIconify_Callback(GLFWwindow* window, int iconified);
};

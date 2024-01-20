#pragma once

#include "../Window/Window.h"

class Events {
public:
    static const short int MOUSE_BUTTONS = 1024;

    static bool keys[1032];

    static float cursor_x;
    static float cursor_y;
    static float cursor_delta_x;
    static float cursor_delta_y;
    static bool cursor_is_moving;
    static bool cursor_is_locked;

    static unsigned long long int current_frame;
    static unsigned long long int frames[1032];


    static void Initialize();
    static void PollEvents();

    static bool KeyIsClicked(int key);
    static bool KeyIsPressed(int key);
    static bool MouseIsClicked(int button);
    static bool MouseIsPressed(int button);
    static void SwitchCursor();
private:
    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void WindowIconifyCallback(GLFWwindow* window, int iconified);
};

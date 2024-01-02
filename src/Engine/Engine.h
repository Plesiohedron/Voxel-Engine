#pragma once

class Engine {
public:
    static void initialize(int width, int height, const char* title);
    static void deinitialize();

    static void mainLoop();
};

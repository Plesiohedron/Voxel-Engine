#pragma once

#include "../GL/Program.h"
#include "../GL/Texture.h"
#include "../GL/VAO.h"

class Engine {
public:
    static void initialize(int width, int height, const char* title);
    static void deinitialize();

    static void mainLoop();
};

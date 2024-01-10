#pragma once

#include "../GL/Program.h"
#include "../GL/Texture.h"
#include "../GL/VAO.h"
#include "../Chunks/Chunk.h"
#include "../GUI/Crosshair.h"

class Engine {
private:
    static glm::vec3 posCamera;
    static float fovCamera;
    static glm::mat4 model;

    static GLint uniformTextureLoc;

    static GLint uniformProjectionLoc;
    static GLint uniformViewLoc;
    static GLint uniformModelLoc;

    static GL::Texture textureAtlas;

public:
    static void initialize(int width, int height, const char* title);
    static void deinitialize();

    static void mainLoop();
};

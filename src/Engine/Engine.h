#pragma once

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/VAO.h"
#include "../Chunks/Chunk.h"
#include "../GUI/Crosshair.h"

class Engine {
private:
    static std::vector<GLushort> vertices;
    static std::vector<GLushort> indexes;

    static glm::vec3 posCamera;
    static float fovCamera;
    static glm::mat4 model;

    static std::unique_ptr<GL::Texture3D> textureAtlas;
    static std::unique_ptr<GL::Program> shaderProgram;

    static GLint uniformTextureLoc;
    static GLint uniformProjectionLoc;
    static GLint uniformViewLoc;
    static GLint uniformModelLoc;
public:
    static void Initialize(int width, int height, const char* title);
    static void MainLoop();
    static void Deinitialize();
};

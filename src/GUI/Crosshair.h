#pragma once

#include "../GL/VAO.h"
#include "../GL/Program.h"
#include "../GL/Texture2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Crosshair {
private:
    static const GLsizei crosshairWidth;
    static const GLsizei crosshairHeight;

    static unsigned windowWidth;
    static unsigned windowHeight;

    static std::unique_ptr<GL::Program> crosshairShader;
    static std::unique_ptr<GL::VAO> crosshairVAO;
    static glm::mat4 model;

    static std::unique_ptr<GL::Texture2D> crosshairTexture;
    static std::unique_ptr<GL::Texture2D> crosshairRegionTexture;
    static GLubyte crosshairRegionColors[3 * 32 * 32];

    static GLuint uniformCrosshairTextureLoc;
    static GLuint uniformCrosshairRegionTextureLoc;
    static GLint uniformModelLoc;

    static GLushort vertices[8];
    static GLushort indexes[6];

    static void MakeCrosshairRegionTexture();

public:
    static void Initialize(int width, int height);
    static void UpdateModel(int width, int height);
    static void Draw(bool cursorIsMoving, bool cursorIsLocked);
};

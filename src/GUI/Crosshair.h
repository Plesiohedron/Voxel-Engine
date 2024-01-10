#pragma once

#include "../GL/VAO.h"
#include "../GL/Program.h"
#include "../GL/Texture.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Crosshair {
private:
    static const GLsizei crosshairWidth = 32;
    static const GLsizei crosshairHeight = 32;

    unsigned windowWidth;
    unsigned windowHeight;

    GL::Program crosshairShader{"Crosshair"};
    GL::VAO crosshairVAO{GL::VAO::Type::VAOcrosshair};
    glm::mat4 model{1.0f};

    GL::Texture crosshairTexture;
    GL::Texture crosshairRegionTexture;
    GLubyte crosshairRegionColors[3 * crosshairWidth * crosshairHeight] = {};

    GLuint uniformCrosshairTextureLoc;
    GLuint uniformCrosshairRegionTextureLoc;
    GLint uniformModelLoc;

    //attributes UV and position
    GLushort vertices[8] = {
        (0 << 10) | (0 << 5) | 0, (0 << 5) | 1,
        (1 << 10) | (0 << 5) | 0, (1 << 5) | 1,
        (0 << 10) | (1 << 5) | 0, (0 << 5) | 0,
        (1 << 10) | (1 << 5) | 0, (1 << 5) | 0,
    };;
    GLushort indexes[6] = {
        0, 1, 2, 2, 1, 3
    };

public:
    Crosshair(int width, int height);
    void makeCrosshairRegionTexture();
    void updateModel(int width, int height);
    void draw(bool cursorIsMoving, bool cursorIsLocked);
};

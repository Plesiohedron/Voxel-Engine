#pragma once

#include "../GL/VAO.h"
#include "../GL/Program.h"
#include "../GL/Texture2D.h"
#include "../Events/Events.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Crosshair {
    friend class GUI;

public:
    void UpdateModel();
    void Draw();

private:
    const float WIDTH_ = 32;
    const float HEIGHT_ = 32;

    std::unique_ptr<GL::Program> shader_;
    std::unique_ptr<GL::VAO> VAO_;
    glm::mat4 model_;

    std::unique_ptr<GL::Texture2D> texture_;
    std::unique_ptr<GL::Texture2D> region_texture_;
    GLubyte region_colors_[3 * 32 * 32]{0};

    GLuint uniform_texture_loc_;
    GLuint uniform_region_texture_loc_;
    GLint uniform_model_loc_;

    const std::vector<glm::vec3> position_ = {
        {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}
    };
    const std::vector<glm::vec2> UV_ = {
        {0.0f, 1.0f / 16}, {1.0f / 16, 1.0f / 16},
        {0.0f, 0.0f}, {1.0f / 16, 0.0f}
    };
    const std::vector<GLushort> indexes_ = {0, 1, 2, 2, 1, 3};

private:
    Crosshair();
    ~Crosshair() = default;

    void MakeCrosshairRegionTexture();
};
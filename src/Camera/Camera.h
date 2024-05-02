#pragma once

#include "../Events/Events.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
    friend class Engine;

public:
    void UpdateVectors();
    void Rotate(const float x, const float y, const float z);

    glm::mat4 GetProjection() const;
    glm::mat4 GetView() const;

public:
    float FOV;

    glm::vec3 position;
    glm::mat4 rotation;

    glm::vec3 vector_up;
    glm::vec3 vector_front;
    glm::vec3 vector_right;

    float camera_rotation_X = 0.0f;
    float camera_rotation_Y = 0.0f;

private:
    Camera(const glm::vec3 pos, const float FOV);
    ~Camera() = default;
};
#pragma once

#include "../Window/Window.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
private:
    static float mFOV;


    static void updateVectors();
public:
    static glm::vec3 position;
    static glm::mat4 rotation;

    static glm::vec3 vectorUp;
    static glm::vec3 vectorFront;
    static glm::vec3 vectorRight;

    static float cameraRotationX;
    static float cameraRotationY;


    static void initialize(glm::vec3 pos, float fov);
    static void rotate(float x, float y, float z);

    static glm::mat4 getProjection() {
        return glm::perspective(mFOV, Window::getAspect(), 0.01f, 150.0f);
    }

    static glm::mat4 getView() {
        return glm::lookAt(position, position + vectorFront, vectorUp);
    }
};

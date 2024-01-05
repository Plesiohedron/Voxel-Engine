#include "Camera.h"

float Camera::mFOV;


glm::vec3 Camera::position;
glm::mat4 Camera::rotation(1.0f);

glm::vec3 Camera::vectorUp;
glm::vec3 Camera::vectorFront;
glm::vec3 Camera::vectorRight;

float Camera::cameraRotationX = 0.0f;
float Camera::cameraRotationY = 0.0f;

void Camera::updateVectors() {
    vectorUp = glm::vec3(rotation * glm::vec4(0, 1, 0, 1));
    vectorFront = glm::vec3(rotation * glm::vec4(0, 0, -1, 1));
    vectorRight = glm::vec3(rotation * glm::vec4(1, 0, 0, 1));
}

void Camera::initialize(glm::vec3 pos, float fov) {
    position = pos;
    mFOV = fov;

    updateVectors();
}

void Camera::rotate(float x, float y, float z) {
    rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));
    rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
    rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));

    updateVectors();
}



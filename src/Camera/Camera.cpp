#include "Camera.h"

Camera::Camera(const glm::vec3 pos, const float FOV) : position(pos), FOV(FOV), rotation(1.0f) {
    UpdateVectors();
}

void Camera::UpdateVectors() {
    vector_up = glm::vec3(rotation * glm::vec4(0, 1, 0, 1));
    vector_front = glm::vec3(rotation * glm::vec4(0, 0, -1, 1));
    vector_right = glm::vec3(rotation * glm::vec4(1, 0, 0, 1));
}

void Camera::Rotate(const float x, const float y, const float z) {
    rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));
    rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
    rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));

    UpdateVectors();
}

glm::mat4 Camera::GetProjection() const {
    return glm::perspective(FOV, Events::window->GetAspect(), 0.01f, 150.0f);
}

glm::mat4 Camera::GetView() const {
    return glm::lookAt(position, position + vector_front, vector_up);
}

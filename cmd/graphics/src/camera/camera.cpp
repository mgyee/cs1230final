#include "camera.h"

// Camera::Camera(glm::vec3 pos, glm::vec3 look, glm::vec3 up,
//                float aspectRatio,
//                float heightAngle, float focalLength, float aperture):
//     viewMatrix(calculateViewMatrix(pos, look, up)), inverseViewMatrix(glm::inverse(viewMatrix)),
//     aspectRatio(aspectRatio), heightAngle(heightAngle), focalLength(focalLength), aperture(aperture)
// {}

Camera::Camera() {}


glm::mat4 Camera::getViewMatrix() const {
    // Optional TODO: implement the getter or make your own design
    // throw std::runtime_error("not implemented");
    return viewMatrix;
}

glm::mat4 Camera::getInverseViewMatrix() const {
    return inverseViewMatrix;
}

void Camera::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
}

float Camera::getAspectRatio() const {
    // Optional TODO: implement the getter or make your own design
    // throw std::runtime_error("not implemented");
    return aspectRatio;
}

glm::mat4 Camera::getProjMatrix() const {
    return projMatrix;
}

void Camera::setProjMatrix(float near, float far) {
    projMatrix = calculateProjMatrix(near, far);
}

void Camera::setViewMatrix(glm::vec3 pos, glm::vec3 look, glm::vec3 up) {
    viewMatrix = calculateViewMatrix(pos, look, up);
    inverseViewMatrix = glm::inverse(viewMatrix);
}

float Camera::getHeightAngle() const {
    // Optional TODO: implement the getter or make your own design
    // throw std::runtime_error("not implemented");
    return heightAngle;
}

void Camera::setHeightAngle(float angle) {
    heightAngle = angle;
}

float Camera::getFocalLength() const {
    // Optional TODO: implement the getter or make your own design
    // throw std::runtime_error("not implemented");
    return focalLength;
}

float Camera::getAperture() const {
    // Optional TODO: implement the getter or make your own design
    // throw std::runtime_error("not implemented");
    return aperture;
}

glm::mat4 Camera::calculateViewMatrix(glm::vec3 pos, glm::vec3 look, glm::vec3 up) const {
    glm::vec3 w = glm::normalize(-look);
    glm::vec3 v = glm::normalize(up - glm::dot(up, w)*w);
    glm::vec3 u = glm::normalize(glm::cross(v, w));
    glm::mat4 M_t = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -pos.x, -pos.y, -pos.z, 1);
    glm::mat4 M_r = glm::mat4(u.x, v.x, w.x, 0, u.y, v.y, w.y, 0, u.z, v.z, w.z, 0, 0, 0, 0, 1);
    return M_r * M_t;
}

glm::mat4 Camera::calculateProjMatrix(float near, float far) const {
    float c = -near / far;
    float widthAngle = aspectRatio * heightAngle;
    glm::mat4 m1(1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, -2, 0,
                 0, 0, -1, 1);
    glm::mat4 m2(1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1.f / (1.f + c), -1,
                 0, 0, -c / (1.f + c), 0);
    glm::mat4 m3(1.f / (far * std::tan(widthAngle / 2.f)), 0, 0, 0,
                 0, 1.f / (far * std::tan(heightAngle / 2.f)), 0, 0,
                 0, 0, 1.f / far, 0,
                 0, 0, 0, 1);

    return m1 * m2 * m3;
}

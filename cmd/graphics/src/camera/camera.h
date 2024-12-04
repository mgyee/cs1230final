#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
private:
    glm::mat4 viewMatrix = glm::mat4(1);
    glm::mat4 inverseViewMatrix = glm::mat4(1);
    glm::mat4 projMatrix = glm::mat4(1);
    float aspectRatio;
    float heightAngle;
    float focalLength;
    float aperture;

    glm::mat4 calculateViewMatrix(glm::vec3 pos, glm::vec3 look, glm::vec3 up) const;
    glm::mat4 calculateProjMatrix(float near, float far) const;
public:
    // Camera(glm::vec3 pos, glm::vec3 look, glm::vec3 up, float aspectRatio,
    //        float heightAngle, float focalLength, float aperture);
    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.

    Camera();

    glm::mat4 getViewMatrix() const;

    glm::mat4 getInverseViewMatrix() const;

    glm::mat4 getProjMatrix() const;

    void setProjMatrix(float near, float far);

    void setAspectRatio(float aspectRatio);

    void setViewMatrix(glm::vec3 pos, glm::vec3 look, glm::vec3 up);

    void setHeightAngle(float angle);

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;
};

#endif // CAMERA_H

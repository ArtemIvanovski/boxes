#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles
    float yaw;
    float pitch;

    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // ArcRotate camera specific
    glm::vec3 target;
    float radius;
    float alpha; // horizontal angle
    float beta;  // vertical angle

    // Limits
    float minRadius;
    float maxRadius;
    float maxBeta;

    bool isArcRotate;

public:
    // Free camera constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    // ArcRotate constructor (similar to Babylon.js)
    Camera(float alpha, float beta, float radius, glm::vec3 target);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect, float near = 0.1f, float far = 1000.0f) const;

    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    // ArcRotate specific methods
    void setTarget(const glm::vec3& newTarget);
    void setRadius(float newRadius);
    void setAlpha(float newAlpha);
    void setBeta(float newBeta);

    void updateArcRotatePosition();

private:
    void updateCameraVectors();
};

#endif //CAMERA_H
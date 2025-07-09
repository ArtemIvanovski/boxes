//
// Created by Pasha on 09.07.2025.
//

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

// Camera.cpp implementation
#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position), worldUp(up), yaw(yaw), pitch(pitch),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f),
      isArcRotate(false) {
    updateCameraVectors();
}

Camera::Camera(float alpha, float beta, float radius, glm::vec3 target)
    : alpha(alpha), beta(beta), radius(radius), target(target),
      movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f),
      minRadius(1.0f), maxRadius(100.0f), maxBeta(glm::radians(85.0f)),
      isArcRotate(true) {
    updateArcRotatePosition();
}

glm::mat4 Camera::getViewMatrix() const {
    if (isArcRotate) {
        return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect, float near, float far) const {
    return glm::perspective(glm::radians(zoom), aspect, near, far);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    if (isArcRotate) return; // ArcRotate camera doesn't use keyboard movement

    float velocity = movementSpeed * deltaTime;
    switch (direction) {
        case FORWARD:  position += front * velocity; break;
        case BACKWARD: position -= front * velocity; break;
        case LEFT:     position -= right * velocity; break;
        case RIGHT:    position += right * velocity; break;
        case UP:       position += up * velocity; break;
        case DOWN:     position -= up * velocity; break;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    if (isArcRotate) {
        alpha += xoffset * mouseSensitivity * 0.01f;
        beta += yoffset * mouseSensitivity * 0.01f;

        if (constrainPitch) {
            if (beta > maxBeta) beta = maxBeta;
            if (beta < 0.1f) beta = 0.1f;
        }

        updateArcRotatePosition();
    } else {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        updateCameraVectors();
    }
}

void Camera::processMouseScroll(float yoffset) {
    if (isArcRotate) {
        radius -= yoffset * 0.5f;
        if (radius < minRadius) radius = minRadius;
        if (radius > maxRadius) radius = maxRadius;
        updateArcRotatePosition();
    } else {
        zoom -= yoffset;
        if (zoom < 1.0f) zoom = 1.0f;
        if (zoom > 45.0f) zoom = 45.0f;
    }
}

void Camera::setTarget(const glm::vec3& newTarget) {
    target = newTarget;
    if (isArcRotate) updateArcRotatePosition();
}

void Camera::setRadius(float newRadius) {
    radius = glm::clamp(newRadius, minRadius, maxRadius);
    if (isArcRotate) updateArcRotatePosition();
}

void Camera::setAlpha(float newAlpha) {
    alpha = newAlpha;
    if (isArcRotate) updateArcRotatePosition();
}

void Camera::setBeta(float newBeta) {
    beta = glm::clamp(newBeta, 0.1f, maxBeta);
    if (isArcRotate) updateArcRotatePosition();
}

void Camera::updateArcRotatePosition() {
    position.x = target.x + radius * sin(beta) * cos(alpha);
    position.y = target.y + radius * cos(beta);
    position.z = target.z + radius * sin(beta) * sin(alpha);
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}


#endif //CAMERA_H

#include "Transform.h"

Transform::Transform()
    : position(0.0f), rotation(0.0f), scale(1.0f) {
}

Transform::Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
    : position(pos), rotation(rot), scale(scl) {
}

glm::mat4 Transform::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations in the order: Scale -> Rotate -> Translate
    model = glm::translate(model, position);

    // Apply rotations (in order: Y, X, Z)
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, scale);

    return model;
}

void Transform::translate(const glm::vec3& offset) {
    position += offset;
}

void Transform::rotate(const glm::vec3& eulerAngles) {
    rotation += eulerAngles;
}

void Transform::setScale(const glm::vec3& newScale) {
    scale = newScale;
}
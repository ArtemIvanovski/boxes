#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Transform();
    Transform(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f));

    glm::mat4 getModelMatrix() const;

    void translate(const glm::vec3& offset);
    void rotate(const glm::vec3& eulerAngles);
    void setScale(const glm::vec3& newScale);
};

#endif //TRANSFORM_H
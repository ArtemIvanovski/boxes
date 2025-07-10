#ifndef MATERIAL_H
#define MATERIAL_H

#pragma once

#include <glm/glm.hpp>

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    // Конструктор по умолчанию с нормальными значениями
    Material();

    // Конструктор с параметрами
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin);

    // Предустановленные материалы
    static Material createMetallic(const glm::vec3& color);
    static Material createPlastic(const glm::vec3& color);
    static Material createRubber(const glm::vec3& color);
};

#endif //MATERIAL_H
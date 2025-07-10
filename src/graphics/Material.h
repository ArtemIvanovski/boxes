//
// Created by Pasha on 09.07.2025.
//

#ifndef MATERIAL_H
#define MATERIAL_H



//
// Created by Pasha on 09.07.2025.
//

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
    Material() :
        ambient(0.3f, 0.3f, 0.3f),    // Увеличенное ambient освещение
        diffuse(0.8f, 0.8f, 0.8f),    // Стандартный диффузный цвет
        specular(0.5f, 0.5f, 0.5f),   // Умеренный specular
        shininess(32.0f) {}

    // Конструктор с параметрами
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin) :
        ambient(amb), diffuse(diff), specular(spec), shininess(shin) {}

    // Предустановленные материалы
    static Material createMetallic(const glm::vec3& color) {
        return Material(
            color * 0.1f,     // Низкое ambient
            color * 0.7f,     // Основной цвет
            glm::vec3(0.8f, 0.8f, 0.8f), // Высокий specular
            64.0f             // Высокая отражаемость
        );
    }

    static Material createPlastic(const glm::vec3& color) {
        return Material(
            color * 0.3f,     // Среднее ambient
            color,            // Основной цвет
            glm::vec3(0.3f, 0.3f, 0.3f), // Низкий specular
            16.0f             // Низкая отражаемость
        );
    }

    static Material createRubber(const glm::vec3& color) {
        return Material(
            color * 0.2f,     // Низкое ambient
            color,            // Основной цвет
            glm::vec3(0.1f, 0.1f, 0.1f), // Очень низкий specular
            4.0f              // Очень низкая отражаемость
        );
    }
};

#endif //MATERIAL_H



#endif //MATERIAL_H

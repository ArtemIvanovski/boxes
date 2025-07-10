#include "Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Scene::Scene() {
    // Инициализация сцены
}

void Scene::loadTruckModel(const std::string& path) {
    try {
        truckModel = std::make_unique<Model>(path);
        std::cout << "Truck model loaded successfully from: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load truck model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::loadWheelModel(const std::string& path) {
    try {
        wheelModel = std::make_unique<Model>(path);
        std::cout << "Wheel model loaded successfully from: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load wheel model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::update(float deltaTime) {
    // Обновление логики сцены
    // Пока ничего не делаем
}

void Scene::render(const Shader& shader) const {
    // Render truck
    if (truckModel) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, -1.25f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setMat4("model", model);

        shader.setBool("use_material_override", false);
        truckModel->draw(shader);
    }

    // Render wheel
    if (wheelModel) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -1.25f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setMat4("model", model);

        shader.setBool("use_material_override", false);
        wheelModel->draw(shader);
    }
}
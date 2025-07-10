#include "Scene.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene() {
    // Constructor
}

void Scene::loadTruckModel(const std::string& path) {
    try {
        std::cout << "Loading truck model..." << std::endl;
        truckModel = std::make_unique<Model>(path);
        std::cout << "Truck model loaded successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading truck model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::loadWheelModel(const std::string& path) {
    try {
        std::cout << "Loading wheel model..." << std::endl;
        wheelModel = std::make_unique<Model>(path);
        std::cout << "Wheel model loaded successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading wheel model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::update(float deltaTime) {
    // Update scene objects
}

void Scene::render(const Shader& shader) const {
    // Render truck
    if (truckModel) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, -1.25f, 0.0f));
        shader.setMat4("model", model);
        shader.setBool("use_material_override", false);
        truckModel->draw(shader);
    }

    // Render wheel
    if (wheelModel) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -1.25f, 0.0f));
        shader.setMat4("model", model);
        shader.setBool("use_material_override", false);
        wheelModel->draw(shader);
    }
}
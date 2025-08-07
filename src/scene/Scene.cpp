#include "Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Scene::setTentOpen(bool open) {
    tentOpen = open;
    // Логика будет обрабатываться в Renderer
}

Scene::Scene() {
    // Материал грузовика (как в BabylonJS StandardMaterial)
    Material truckMaterial;
    truckMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // diffuseColor как в BabylonJS
    truckMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    truckMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    truckMaterial.shininess = 32.0f;
    
    // Материал пола (как в BabylonJS)
    Material floorMaterial;
    floorMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    floorMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    floorMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    floorMaterial.shininess = 32.0f;
    
    // Материал земли (как GridMaterial в BabylonJS)
    Material groundMaterial;
    groundMaterial.diffuse = glm::vec3(0.35f, 0.35f, 0.35f); // clearColor как в BabylonJS
    groundMaterial.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    groundMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
    groundMaterial.shininess = 16.0f;

    // Создаем примитивы
    truckBox = Primitives::createBox(truckSize.width, truckSize.height, truckSize.depth, truckMaterial);
    floorBox = Primitives::createBox(truckSize.width, 0.1f, truckSize.depth, floorMaterial);
    ground = Primitives::createGround(48.0f, 48.0f, groundMaterial);
    
    // Создаем менеджер коробок
    boxManager = std::make_unique<BoxManager>();
}

void Scene::updateTruckSize(float width, float height, float depth) {
    truckSize.width = width / 100.0f;   // конвертируем см в метры
    truckSize.height = height / 100.0f;
    truckSize.depth = depth / 100.0f;

    // Пересоздаем примитивы с новыми размерами
    Material truckMaterial;
    truckMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    truckMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    truckMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    truckMaterial.shininess = 32.0f;

    Material floorMaterial;
    floorMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    floorMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    floorMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    floorMaterial.shininess = 32.0f;

    truckBox = Primitives::createBox(truckSize.width, truckSize.height, truckSize.depth, truckMaterial);
    floorBox = Primitives::createBox(truckSize.width, 0.1f, truckSize.depth, floorMaterial);
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
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setBool("use_material_override", false);
    ground->draw(shader);

    // Render truck box (полупрозрачный)
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, truckSize.height / 2.0f, 0.0f));
    shader.setMat4("model", model);
    shader.setBool("use_material_override", false);

    // Включаем blending для прозрачности
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Отключаем запись в буфер глубины для прозрачных объектов
    
    // Устанавливаем прозрачность в зависимости от состояния тента
    if (tentOpen) {
        shader.setFloat("alpha", 0.0f); // Полностью прозрачный
    } else {
        shader.setFloat("alpha", 0.3f); // Полупрозрачный
    }
    shader.setBool("useAlpha", true);
    
    truckBox->draw(shader);
    glDepthMask(GL_TRUE); // Включаем обратно

    // Render floor
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.05f, 0.0f));
    shader.setMat4("model", model);
    floorBox->draw(shader);

    glDisable(GL_BLEND);

    // Render models if they exist
    if (truckModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, -1.25f, 0.0f));
        shader.setMat4("model", model);
        truckModel->draw(shader);
    }

    if (wheelModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, -1.25f, 0.0f));
        shader.setMat4("model", model);
        wheelModel->draw(shader);
    }

    // Рендерим коробки
    if (boxManager) {
        boxManager->renderBoxes(shader);
    }
}
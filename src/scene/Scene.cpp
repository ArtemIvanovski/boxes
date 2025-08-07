#include "Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Scene::setTentOpen(bool open) {
    tentOpen = open;
}

Scene::Scene() {
    Material truckMaterial;
    truckMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // diffuseColor как в BabylonJS
    truckMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    truckMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    truckMaterial.shininess = 32.0f;

    Material floorMaterial;
    floorMaterial.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    floorMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    floorMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    floorMaterial.shininess = 32.0f;

    Material groundMaterial;
    groundMaterial.diffuse = glm::vec3(0.35f, 0.35f, 0.35f); // clearColor как в BabylonJS
    groundMaterial.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    groundMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
    groundMaterial.shininess = 16.0f;

    truckBox = Primitives::createBox(truckSize.width, truckSize.height, truckSize.depth, truckMaterial);
    floorBox = Primitives::createBox(truckSize.width, 0.1f, truckSize.depth, floorMaterial);
    ground = Primitives::createGround(48.0f, 48.0f, groundMaterial);

    boxManager = std::make_unique<BoxManager>();
}

void Scene::updateTruckSize(float width, float height, float depth) {
    truckSize.width = width / 100.0f; // конвертируем см в метры
    truckSize.height = height / 100.0f;
    truckSize.depth = depth / 100.0f;

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

void Scene::loadTruckModel(const std::string &path) {
    try {
        truckModel = std::make_unique<Model>(path);
        std::cout << "Truck model loaded successfully from: " << path << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Failed to load truck model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::loadWheelModel(const std::string &path) {
    try {
        wheelModel = std::make_unique<Model>(path);
        std::cout << "Wheel model loaded successfully from: " << path << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Failed to load wheel model: " << e.what() << std::endl;
        throw;
    }
}

void Scene::update(float deltaTimeParam) {
    (void)deltaTimeParam; // Suppress unused parameter warning
    // Обновление логики сцены
    updateLoadZones();
}

void Scene::render(const Shader &shader) const {
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setBool("use_material_override", false);
    ground->draw(shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, truckSize.height / 2.0f, 0.0f));
    shader.setMat4("model", model);
    shader.setBool("use_material_override", false);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    if (tentOpen) {
        shader.setFloat("alpha", 0.0f);
    } else {
        shader.setFloat("alpha", 0.3f);
    }
    shader.setBool("useAlpha", true);

    truckBox->draw(shader);
    glDepthMask(GL_TRUE);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.05f, 0.0f));
    shader.setMat4("model", model);
    floorBox->draw(shader);

    glDisable(GL_BLEND);

    if (truckModel) {
        float lorryPosition = -400.0f / 100.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(lorryPosition + truckSize.width / 2.0f, -1.25f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, -1.0f, -1.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setMat4("model", model);
        truckModel->draw(shader);
    }

    if (wheelModel) {
        float wheelPosition = 50.0f / 100.0f;
        float truckLong = truckSize.width < 10.0f ? 10.0f : truckSize.width;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(wheelPosition - truckLong / 2.0f, -1.25f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, -1.0f, -1.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setMat4("model", model);
        wheelModel->draw(shader);
    }

    if (boxManager) {
        boxManager->renderBoxes(shader);
    }
}

void Scene::updateLoadZones() {
    const int sectionCount = 4;
    float sectionLength = truckSize.width / sectionCount;

    loadZones.clear();
    loadZones.resize(sectionCount);

    for (int i = 0; i < sectionCount; i++) {
        loadZones[i].zoneIndex = i;
        loadZones[i].weight = 0.0f;
        loadZones[i].position.x = -truckSize.width / 2.0f + sectionLength * (i + 0.5f);
        loadZones[i].position.y = truckSize.height + 0.5f;
        loadZones[i].position.z = 0.0f;
    }

    // Пересчитываем вес в каждой зоне
    if (boxManager) {
        const auto &boxes = boxManager->getBoxes();
        for (const auto &box: boxes) {
            if (box.isOnScene) {
                // Проверяем, в какой зоне находится коробка
                for (int i = 0; i < sectionCount; i++) {
                    float sectionStart = -truckSize.width / 2.0f + sectionLength * i;
                    float sectionEnd = sectionStart + sectionLength;

                    if (box.position.x >= sectionStart && box.position.x < sectionEnd) {
                        // Проверяем, что коробка внутри грузовика
                        bool insideTruck = (box.position.x >= -truckSize.width / 2.0f &&
                                            box.position.x <= truckSize.width / 2.0f &&
                                            box.position.y >= 0.0f &&
                                            box.position.y <= truckSize.height &&
                                            box.position.z >= -truckSize.depth / 2.0f &&
                                            box.position.z <= truckSize.depth / 2.0f);

                        if (insideTruck) {
                            loadZones[i].weight += box.weight;
                        }
                        break;
                    }
                }
            }
        }
    }
}

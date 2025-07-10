#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <memory>
#include <vector>
#include <string>
#include "../graphics/Model.h"
#include "../graphics/Shader.h"

class Scene {
private:
    std::unique_ptr<Model> truckModel;
    std::unique_ptr<Model> wheelModel;

public:
    Scene();
    ~Scene() = default;

    void loadTruckModel(const std::string& path);
    void loadWheelModel(const std::string& path);

    void update(float deltaTime);
    void render(const Shader& shader) const;

    // Getters
    Model* getTruckModel() const { return truckModel.get(); }
    Model* getWheelModel() const { return wheelModel.get(); }
};

#endif //SCENE_H
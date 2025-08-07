#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <memory>
#include <vector>
#include <string>
#include "../graphics/Model.h"
#include "../graphics/Shader.h"
#include "../graphics/Primitives.h"
#include "BoxManager.h"

class Scene {
private:
    bool tentOpen = false;
    std::unique_ptr<Model> truckModel;
    std::unique_ptr<Model> wheelModel;

    std::unique_ptr<Mesh> truckBox;
    std::unique_ptr<Mesh> floorBox;
    std::unique_ptr<Mesh> ground;
    std::unique_ptr<BoxManager> boxManager;

    bool modelPositionsNeedUpdate = false;
    struct LoadZone {
        int zoneIndex;
        float weight;
        glm::vec3 position;
    };
    std::vector<LoadZone> loadZones;
    void updateLoadZones();

    struct TruckDimensions {
        float width = 16.5f;   // 1650 см в масштабе
        float height = 2.6f;   // 260 см в масштабе
        float depth = 2.45f;   // 245 см в масштабе
    } truckSize;

public:
    Scene();
    ~Scene() = default;

    void setTentOpen(bool open);
    void loadTruckModel(const std::string& path);
    void loadWheelModel(const std::string& path);
    void updateTruckSize(float width, float height, float depth);

    void update(float deltaTime);
    void render(const Shader& shader) const;

    // Getters
    Model* getTruckModel() const { return truckModel.get(); }
    Model* getWheelModel() const { return wheelModel.get(); }
    const TruckDimensions& getTruckSize() const { return truckSize; }
    BoxManager* getBoxManager() { return boxManager.get(); }
    const BoxManager* getBoxManager() const { return boxManager.get(); }
};

#endif //SCENE_H
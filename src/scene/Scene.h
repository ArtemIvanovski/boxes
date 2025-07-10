#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>
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
    const Model* getTruckModel() const { return truckModel.get(); }
    const Model* getWheelModel() const { return wheelModel.get(); }
};

#endif // SCENE_H
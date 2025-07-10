#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>
#include <memory>
#include "Transform.h"
#include "../graphics/Model.h"

class GameObject {
public:
    std::string name;
    Transform transform;
    bool active;

private:
    std::shared_ptr<Model> model;

public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject() = default;

    void setModel(std::shared_ptr<Model> newModel);
    Model* getModel() const;

    void update(float deltaTime);
    void render(const Shader& shader) const;

    void setActive(bool isActive);
    bool isActive() const;
};

#endif //GAMEOBJECT_H
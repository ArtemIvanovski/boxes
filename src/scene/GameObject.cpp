#include "GameObject.h"

GameObject::GameObject(const std::string& name)
    : name(name), active(true) {
}

void GameObject::setModel(std::shared_ptr<Model> newModel) {
    model = newModel;
}

Model* GameObject::getModel() const {
    return model.get();
}

void GameObject::update(float deltaTime) {
    if (!active) return;

    // Update game object logic here
    // For now, do nothing
}

void GameObject::render(const Shader& shader) const {
    if (!active || !model) return;

    // Set model matrix
    shader.setMat4("model", transform.getModelMatrix());

    // Render the model
    model->draw(shader);
}

void GameObject::setActive(bool isActive) {
    active = isActive;
}

bool GameObject::isActive() const {
    return active;
}
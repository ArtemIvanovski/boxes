#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "Window.h"
#include "Renderer.h"
#include "../scene/Scene.h"
#include "../graphics/Camera.h"

class Application {
private:
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Camera> camera;

    // Timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Camera control
    bool cameraControlEnabled = true;
    bool firstMouse = true;
    float lastX = 960.0f;
    float lastY = 540.0f;

    // Settings
    bool running = true;

    void initializeSubsystems();
    void setupCamera();
    void setupCallbacks();
    void update(float deltaTime);
    void render();
    void cleanup();

public:
    Application();
    ~Application();

    void run();
    void shutdown();
};

#endif // APPLICATION_H
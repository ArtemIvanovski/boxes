#include "Application.h"
#include <iostream>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Application::Application() {
    initializeSubsystems();
    setupCamera();
}

Application::~Application() {
    cleanup();
}

void Application::initializeSubsystems() {
    // Initialize window
    window = std::make_unique<Window>(1920, 1080, "Truck Loading Simulator");

    // Initialize renderer
    renderer = std::make_unique<Renderer>();

    // Initialize UI after window creation
    renderer->initializeUI(window->getGLFWWindow());

    // Initialize scene
    scene = std::make_unique<Scene>();

    // Load models
    try {
        scene->loadTruckModel("assets/models/lorry.obj");
        scene->loadWheelModel("assets/models/weel.obj");
    } catch (const std::exception& e) {
        std::cerr << "Error loading models: " << e.what() << std::endl;
        throw;
    }

    // Setup callbacks
    setupCallbacks();
}

void Application::setupCamera() {
    camera = std::make_unique<Camera>(
        glm::radians(90.0f),    // alpha
        glm::radians(60.0f),    // beta
        20.0f,                  // radius
        glm::vec3(0.0f, 3.0f, 0.0f)  // target
    );
    camera->minRadius = 1.5f;
    camera->maxRadius = 50.0f;
}

void Application::setupCallbacks() {
    // Mouse callback
    window->setMouseCallback([this](double xpos, double ypos) {
        if (!cameraControlEnabled) return;

        if (firstMouse) {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos);

        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            camera->processMouseMovement(xoffset, yoffset);
        }
    });

    // Scroll callback
    window->setScrollCallback([this](double xoffset, double yoffset) {
        if (cameraControlEnabled) {
            camera->processMouseScroll(static_cast<float>(yoffset));
        }
    });

    // Key callback
    window->setKeyCallback([this](int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            shutdown();
        }

        if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
            cameraControlEnabled = !cameraControlEnabled;
        }

        // Camera presets
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_1: // Top view
                    camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                    camera->setRadius(20.0f);
                    camera->setAlpha(glm::radians(90.0f));
                    camera->setBeta(glm::radians(5.0f));
                    break;
                case GLFW_KEY_2: // Left view
                    camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                    camera->setRadius(20.0f);
                    camera->setAlpha(glm::radians(0.0f));
                    camera->setBeta(glm::radians(90.0f));
                    break;
                case GLFW_KEY_3: // Right view
                    camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                    camera->setRadius(20.0f);
                    camera->setAlpha(glm::radians(180.0f));
                    camera->setBeta(glm::radians(90.0f));
                    break;
                case GLFW_KEY_4: // Isometric view
                    camera->setTarget(glm::vec3(0.0f, 3.0f, 0.0f));
                    camera->setRadius(20.0f);
                    camera->setAlpha(glm::radians(45.0f));
                    camera->setBeta(glm::radians(60.0f));
                    break;
            }
        }
    });

    // Resize callback
    window->setResizeCallback([this](int width, int height) {
        // Handle window resize
    });
}

void Application::run() {
    while (running && !window->shouldClose()) {
        // Timing
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update
        window->pollEvents();
        update(deltaTime);

        // Render
        render();

        window->swapBuffers();
    }
}

void Application::update(float deltaTime) {
    // Update scene
    scene->update(deltaTime);

    // Check for exit
    if (window->isKeyPressed(GLFW_KEY_ESCAPE)) {
        shutdown();
    }
}

void Application::render() {
    renderer->clear();
    renderer->render(*scene, *camera);
    renderer->renderUI(*scene, window->getGLFWWindow());
}

void Application::shutdown() {
    running = false;
}

void Application::cleanup() {
    // Cleanup handled by smart pointers
    // But we need to cleanup UI manually
    if (renderer) {
        renderer->cleanupUI();
    }
}
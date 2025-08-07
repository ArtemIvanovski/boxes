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

    renderer->setScene(scene.get());
    // Load models (GLB files)
    try {
        scene->loadTruckModel("assets/models/lorry.glb");
        scene->loadWheelModel("assets/models/weel.glb");
    } catch (const std::exception& e) {
        std::cerr << "Error loading models: " << e.what() << std::endl;
        // Fallback to OBJ files if GLB not available
        try {
            scene->loadTruckModel("assets/models/lorry.obj");
            scene->loadWheelModel("assets/models/weel.obj");
        } catch (const std::exception& e2) {
            std::cerr << "Error loading fallback models: " << e2.what() << std::endl;
            throw;
        }
    }

    // Setup callbacks
    setupCallbacks();
}

void Application::setupCamera() {
    // Настройки камеры точно как в BabylonJS
    camera = std::make_unique<Camera>(
        glm::radians(90.0f),    // alpha = Math.PI / 2
        glm::radians(60.0f),    // beta = Math.PI / 3
        20.0f,                  // radius = 2000 (масштабированный)
        glm::vec3(0.0f, 3.0f, 0.0f)  // target = (0, 300, 0) (масштабированный)
    );
    camera->minRadius = 1.5f;   // 150 масштабированный
    camera->maxRadius = 35.0f;  // 3500 масштабированный
    camera->maxBeta = glm::radians(89.1f); // (Math.PI / 2) * 0.99
}

void Application::setupCallbacks() {
    // Mouse callback
    window->setMouseCallback([this](double xpos, double ypos) {
        if (firstMouse) {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos);

        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        // Handle BoxManager mouse input first
        if (scene && scene->getBoxManager()) {
            bool leftPressed = window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
            bool rightPressed = window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
            scene->getBoxManager()->handleMouseInput(xpos, ypos, leftPressed, rightPressed);
        }

        // Правая кнопка мыши - движение камеры вдоль сцены (как в BabylonJS)
        if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            camera->processMouseMovement(xoffset, yoffset);
        }
        
        // Левая кнопка мыши - вращение грузовика (как в BabylonJS)
        if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            // TODO: Implement truck rotation with left click
            // This should rotate the truck model when left-clicking and dragging
        }
    });

    // Scroll callback
    window->setScrollCallback([this](double xoffset, double yoffset) {
        (void)xoffset; // Suppress unused parameter warning
        if (cameraControlEnabled) {
            camera->processMouseScroll(static_cast<float>(yoffset));
        }
    });

    // Keyboard callback
    window->setKeyCallback([this](int key, int scancode, int action, int mods) {
        (void)scancode; // Suppress unused parameter warning
        (void)mods; // Suppress unused parameter warning
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    window->close();
                    break;
                case GLFW_KEY_F11:
                    window->toggleFullscreen();
                    break;
            }
        }
        
        // Handle BoxManager keyboard input
        if (scene && scene->getBoxManager()) {
            scene->getBoxManager()->handleKeyInput(key, action);
        }
    });

    glfwSetKeyCallback(window->getGLFWWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; // Suppress unused parameter warning
    (void)mods; // Suppress unused parameter warning
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_TAB:
                if (app->scene && app->scene->getBoxManager()) {
                    app->scene->getBoxManager()->toggleBoxPanel();
                }
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
        }
    }

    // Передаем клавиши BoxManager
    if (app->scene && app->scene->getBoxManager()) {
        app->scene->getBoxManager()->handleKeyInput(key, action);
    }
});

    // Resize callback
    window->setResizeCallback([this](int width, int height) {
        (void)width; // Suppress unused parameter warning
        (void)height; // Suppress unused parameter warning
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

void Application::update(float deltaTimeParam) {
    // Update scene
    scene->update(deltaTimeParam);

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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Исправленные пути для ImGui - убираем imgui/ из начала
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <memory>
#include <GL/gl.h>

#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Model.h"


// Global variables
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera
std::unique_ptr<Camera> camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraControlEnabled = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Material override settings
struct MaterialOverride {
    // ИСПРАВЛЕНО: По умолчанию отключаем переопределение, чтобы показывать натуральные цвета
    bool useTruckOverride = false;  // Изменено с true на false
    glm::vec3 truckColor = glm::vec3(0.8f, 0.2f, 0.2f); // Красный цвет для грузовика
    bool useWheelOverride = false;  // Изменено с true на false
    glm::vec3 wheelColor = glm::vec3(0.1f, 0.1f, 0.1f); // Черный цвет для колес

    // Настройки освещения
    glm::vec3 ambientStrength = glm::vec3(0.3f, 0.3f, 0.3f); // Увеличенное ambient освещение
    glm::vec3 lightColor = glm::vec3(1.2f, 1.2f, 1.0f); // Более яркий свет с теплым оттенком
    glm::vec3 lightPos = glm::vec3(10.0f, 15.0f, 10.0f);

    // Дополнительные настройки для улучшения отображения материалов
    float materialBrightness = 1.0f;  // Множитель яркости материала
    bool enhanceContrast = true;      // Улучшение контраста
};

MaterialOverride materialSettings;

// Performance monitoring
struct PerformanceMetrics {
    float frameTime = 0.0f;
    float fps = 0.0f;
    int triangleCount = 0;
    int drawCalls = 0;

    void update(float dt, int triangles, int calls) {
        frameTime = dt * 1000.0f; // Convert to ms
        fps = 1.0f / dt;
        triangleCount = triangles;
        drawCalls = calls;
    }
};

PerformanceMetrics metrics;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void renderUI();
void setupOpenGLSettings();

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Truck Loading Simulator", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup OpenGL settings for optimization
    setupOpenGLSettings();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Initialize camera (ArcRotate like in Babylon.js)
    camera = std::make_unique<Camera>(
        glm::radians(90.0f),    // alpha (horizontal angle)
        glm::radians(60.0f),    // beta (vertical angle)
        20.0f,                   // radius
        glm::vec3(0.0f, 3.0f, 0.0f)  // target
    );
    camera->minRadius = 1.5f;
    camera->maxRadius = 50.0f;

    // Load shaders
    Shader modelShader("assets/shaders/model.vs", "assets/shaders/model.fs");

    // Load models
    std::unique_ptr<Model> truckModel;
    std::unique_ptr<Model> wheelModel;

    try {
        std::cout << "Loading truck model..." << std::endl;
        truckModel = std::make_unique<Model>("assets/models/lorry.obj");

        std::cout << "Loading wheel model..." << std::endl;
        wheelModel = std::make_unique<Model>("assets/models/weel.obj");

        std::cout << "Models loaded successfully!" << std::endl;
        std::cout << "Truck - Triangles: " << truckModel->getTriangleCount()
                  << ", Vertices: " << truckModel->getVertexCount() << std::endl;
        std::cout << "Wheel - Triangles: " << wheelModel->getTriangleCount()
                  << ", Vertices: " << wheelModel->getVertexCount() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading models: " << e.what() << std::endl;
        std::cerr << "Make sure .obj and .mtl files are in the assets/models/ directory" << std::endl;
        return -1;
    }

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        modelShader.use();

        // View/projection transformations
        glm::mat4 projection = camera->getProjectionMatrix(
            (float)SCR_WIDTH / (float)SCR_HEIGHT);
        glm::mat4 view = camera->getViewMatrix();

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        // Enhanced lighting setup
        modelShader.setVec3("lightPos", materialSettings.lightPos);
        modelShader.setVec3("lightColor", materialSettings.lightColor);
        modelShader.setVec3("viewPos", camera->position);
        modelShader.setVec3("ambientStrength", materialSettings.ambientStrength);

        // Добавляем настройки для улучшения отображения материалов
        modelShader.setFloat("materialBrightness", materialSettings.materialBrightness);
        modelShader.setBool("enhanceContrast", materialSettings.enhanceContrast);

        int drawCalls = 0;
        int totalTriangles = 0;

        // Render truck
        if (truckModel) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.0f, -1.25f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            modelShader.setMat4("model", model);

            // Используем натуральные цвета из .mtl файла или переопределяем
            if (materialSettings.useTruckOverride) {
                modelShader.setBool("use_material_override", true);
                modelShader.setVec3("material_override_diffuse", materialSettings.truckColor);
            } else {
                modelShader.setBool("use_material_override", false);
            }

            truckModel->draw(modelShader);
            drawCalls++;
            totalTriangles += truckModel->getTriangleCount();
        }

        // Render wheel
        if (wheelModel) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.5f, -1.25f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            modelShader.setMat4("model", model);

            // Используем натуральные цвета из .mtl файла или переопределяем
            if (materialSettings.useWheelOverride) {
                modelShader.setBool("use_material_override", true);
                modelShader.setVec3("material_override_diffuse", materialSettings.wheelColor);
            } else {
                modelShader.setBool("use_material_override", false);
            }

            wheelModel->draw(modelShader);
            drawCalls++;
            totalTriangles += wheelModel->getTriangleCount();
        }

        // Update performance metrics
        metrics.update(deltaTime, totalTriangles, drawCalls);

        // Render UI
        renderUI();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!cameraControlEnabled) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Only process mouse movement when right mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (cameraControlEnabled) {
        camera->processMouseScroll(static_cast<float>(yoffset));
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle camera control with F1
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        cameraControlEnabled = !cameraControlEnabled;
        if (cameraControlEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    // Быстрое переключение между натуральными и переопределенными цветами
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        materialSettings.useTruckOverride = !materialSettings.useTruckOverride;
        materialSettings.useWheelOverride = !materialSettings.useWheelOverride;
        std::cout << "Material override: " << (materialSettings.useTruckOverride ? "ON" : "OFF") << std::endl;
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void renderUI() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Performance window
    ImGui::Begin("Performance Metrics");
    ImGui::Text("FPS: %.1f", metrics.fps);
    ImGui::Text("Frame Time: %.3f ms", metrics.frameTime);
    ImGui::Text("Triangles: %d", metrics.triangleCount);
    ImGui::Text("Draw Calls: %d", metrics.drawCalls);

    ImGui::Separator();
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
                camera->position.x, camera->position.y, camera->position.z);
    ImGui::Text("Camera Target: (%.2f, %.2f, %.2f)",
                camera->target.x, camera->target.y, camera->target.z);
    ImGui::Text("Camera Radius: %.2f", camera->radius);

    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::Text("Right Mouse + Move: Rotate camera");
    ImGui::Text("Mouse Wheel: Zoom in/out");
    ImGui::Text("F1: Toggle camera control");
    ImGui::Text("M: Toggle material override");  // Добавлено
    ImGui::Text("ESC: Exit");

    ImGui::End();

    // Camera controls window
    ImGui::Begin("Camera Controls");

    static float target[3] = {camera->target.x, camera->target.y, camera->target.z};
    if (ImGui::SliderFloat3("Target", target, -10.0f, 10.0f)) {
        camera->setTarget(glm::vec3(target[0], target[1], target[2]));
    }

    float radius = camera->radius;
    if (ImGui::SliderFloat("Radius", &radius, camera->minRadius, camera->maxRadius)) {
        camera->setRadius(radius);
    }

    float alpha = glm::degrees(camera->alpha);
    if (ImGui::SliderFloat("Alpha (degrees)", &alpha, 0.0f, 360.0f)) {
        camera->setAlpha(glm::radians(alpha));
    }

    float beta = glm::degrees(camera->beta);
    if (ImGui::SliderFloat("Beta (degrees)", &beta, 1.0f, 89.0f)) {
        camera->setBeta(glm::radians(beta));
    }

    if (ImGui::Button("Reset Camera")) {
        camera->setTarget(glm::vec3(0.0f, 3.0f, 0.0f));
        camera->setRadius(20.0f);
        camera->setAlpha(glm::radians(90.0f));
        camera->setBeta(glm::radians(60.0f));
    }

    ImGui::End();

    // УЛУЧШЕННОЕ: Material Settings Window
    ImGui::Begin("Material & Lighting Settings");

    // Режим отображения материалов
    ImGui::Text("Material Display Mode:");
    ImGui::Separator();

    if (ImGui::Button("Use Natural Colors (.mtl)")) {
        materialSettings.useTruckOverride = false;
        materialSettings.useWheelOverride = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Use Custom Colors")) {
        materialSettings.useTruckOverride = true;
        materialSettings.useWheelOverride = true;
    }

    ImGui::Spacing();

    // Truck Material
    ImGui::Text("Truck Material:");
    ImGui::Checkbox("Override Truck Color", &materialSettings.useTruckOverride);
    if (materialSettings.useTruckOverride) {
        ImGui::ColorEdit3("Truck Color", &materialSettings.truckColor[0]);
    }

    ImGui::Separator();

    // Wheel Material
    ImGui::Text("Wheel Material:");
    ImGui::Checkbox("Override Wheel Color", &materialSettings.useWheelOverride);
    if (materialSettings.useWheelOverride) {
        ImGui::ColorEdit3("Wheel Color", &materialSettings.wheelColor[0]);
    }

    ImGui::Separator();

    // Material Enhancement
    ImGui::Text("Material Enhancement:");
    ImGui::SliderFloat("Material Brightness", &materialSettings.materialBrightness, 0.1f, 3.0f);
    ImGui::Checkbox("Enhance Contrast", &materialSettings.enhanceContrast);

    ImGui::Separator();

    // Lighting Settings
    ImGui::Text("Lighting Settings:");
    ImGui::ColorEdit3("Ambient Strength", &materialSettings.ambientStrength[0]);
    ImGui::ColorEdit3("Light Color", &materialSettings.lightColor[0]);
    ImGui::SliderFloat3("Light Position", &materialSettings.lightPos[0], -20.0f, 20.0f);

    ImGui::Separator();

    if (ImGui::Button("Reset to Natural Colors")) {
        materialSettings.useTruckOverride = false;
        materialSettings.useWheelOverride = false;
        materialSettings.materialBrightness = 1.0f;
        materialSettings.enhanceContrast = true;
        materialSettings.ambientStrength = glm::vec3(0.3f, 0.3f, 0.3f);
        materialSettings.lightColor = glm::vec3(1.2f, 1.2f, 1.0f);
        materialSettings.lightPos = glm::vec3(10.0f, 15.0f, 10.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset to Custom Colors")) {
        materialSettings.useTruckOverride = true;
        materialSettings.useWheelOverride = true;
        materialSettings.truckColor = glm::vec3(0.8f, 0.2f, 0.2f);
        materialSettings.wheelColor = glm::vec3(0.1f, 0.1f, 0.1f);
        materialSettings.materialBrightness = 1.0f;
        materialSettings.enhanceContrast = true;
        materialSettings.ambientStrength = glm::vec3(0.3f, 0.3f, 0.3f);
        materialSettings.lightColor = glm::vec3(1.2f, 1.2f, 1.0f);
        materialSettings.lightPos = glm::vec3(10.0f, 15.0f, 10.0f);
    }

    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void setupOpenGLSettings() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable face culling for better performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable multisampling (MSAA)
    glEnable(GL_MULTISAMPLE);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Print OpenGL information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Graphics Card: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>
#include <memory>


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
        truckModel = std::make_unique<Model>("assets/models/lorry.glb");
        wheelModel = std::make_unique<Model>("assets/models/weel.glb");

        std::cout << "Truck model loaded successfully!" << std::endl;
        std::cout << "Triangles: " << truckModel->getTriangleCount() << std::endl;
        std::cout << "Vertices: " << truckModel->getVertexCount() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading models: " << e.what() << std::endl;
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

        // Lighting setup
        modelShader.setVec3("lightPos", glm::vec3(10.0f, 10.0f, 10.0f));
        modelShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        modelShader.setVec3("viewPos", camera->position);

        int drawCalls = 0;
        int totalTriangles = 0;

        // Render truck
        if (truckModel) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-4.0f, -1.25f, 0.0f));
            model = glm::scale(model, glm::vec3(-1.0f, -1.0f, -1.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelShader.setMat4("model", model);

            truckModel->draw(modelShader);
            drawCalls++;
            totalTriangles += truckModel->getTriangleCount();
        }

        // Render wheel
        if (wheelModel) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.5f, -1.25f, 0.0f));
            model = glm::scale(model, glm::vec3(-1.0f, -1.0f, -1.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            modelShader.setMat4("model", model);

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
#include "Renderer.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <array>

Renderer::Renderer() {
    // Initialize shaders
    modelShader = std::make_unique<Shader>("assets/shaders/model.vs", "assets/shaders/model.fs");


    // Set lighting - как HemisphericLight в BabylonJS
    modelShader->setVec3("lightPos", glm::vec3(-1.0f, 1.0f, -1.0f));
    modelShader->setVec3("lightColor", glm::vec3(1.5f, 1.5f, 1.5f)); // intensity 1.5
    modelShader->setVec3("ambientStrength", glm::vec3(0.4f, 0.4f, 0.4f));
    
    // Для прозрачности (как в BabylonJS)
    modelShader->setFloat("alpha", 0.3f);
    modelShader->setBool("useAlpha", true);

    // Initialize truck presets
    truckPresets = {
        {"Малый грузовик", 590, 239, 235},
        {"Компактный грузовик", 1203, 239, 235},
        {"Стандартный грузовик", 1340, 239, 235},
        {"Средний грузовик", 1360, 260, 245},
        {"Увеличенный грузовик", 1360, 300, 245},
        {"Большой грузовик", 1650, 260, 245}
    };
}

Renderer::~Renderer() {
    cleanupUI();
}

void Renderer::setScene(Scene* scene) {
    currentScene = scene;
}

void Renderer::initializeUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Try to load Russian font
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (font == nullptr) {
        io.Fonts->AddFontDefault();
        std::cout << "Warning: Could not load custom font, using default" << std::endl;
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Renderer::clear() {
    glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(const Scene& scene, const Camera& camera) {
    modelShader->use();

    // Set up matrices
    glm::mat4 projection = camera.getProjectionMatrix(1920.0f / 1080.0f);
    glm::mat4 view = camera.getViewMatrix();

    modelShader->setMat4("projection", projection);
    modelShader->setMat4("view", view);

    // Set lighting (как в BabylonJS HemisphericLight)
    modelShader->setVec3("lightPos", glm::vec3(-1.0f, 1.0f, -1.0f));
    modelShader->setVec3("lightColor", glm::vec3(1.5f, 1.5f, 1.5f)); // intensity 1.5 как в BabylonJS
    modelShader->setVec3("viewPos", camera.position);
    modelShader->setVec3("ambientStrength", glm::vec3(0.4f, 0.4f, 0.4f));

    // Render scene
    scene.render(*modelShader);
}

void Renderer::renderUI(const Scene& scene, GLFWwindow* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderMainMenuBar(window);
    renderTruckInfoPanel(scene);
    renderPerformancePanel();
    
    // Рендерим панель коробок
    auto* boxManager = const_cast<Scene&>(scene).getBoxManager();
    if (boxManager) {
        boxManager->renderBoxPanel();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::renderMainMenuBar(GLFWwindow* window) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Файл")) {
            if (ImGui::MenuItem("Новый проект", "Ctrl+N")) {
                // Implementation
            }
            if (ImGui::MenuItem("Открыть", "Ctrl+O")) {
                // Implementation
            }
            if (ImGui::MenuItem("Сохранить", "Ctrl+S")) {
                // Implementation
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Выход", "Alt+F4")) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Грузовик")) {
            ImGui::Text("Тип прицепа:");
            ImGui::Separator();

            for (int i = 0; i < truckPresets.size(); i++) {
                bool selected = (truckSettings.currentPreset == i && !truckSettings.useCustom);
                if (ImGui::MenuItem(truckPresets[i].name.c_str(), nullptr, selected)) {
                    truckSettings.currentPreset = i;
                    truckSettings.useCustom = false;
                    updateTruckSize();
                }
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Пользовательский", nullptr, truckSettings.useCustom)) {
                truckSettings.useCustom = true;
                updateTruckSize();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Параметры")) {
            ImGui::Text("Размеры прицепа (см):");
            ImGui::PushItemWidth(100);

            bool changed = false;
            if (truckSettings.useCustom) {
                changed |= ImGui::InputInt("Ширина", &truckSettings.customWidth, 10, 100);
                changed |= ImGui::InputInt("Высота", &truckSettings.customHeight, 10, 100);
                changed |= ImGui::InputInt("Глубина", &truckSettings.customDepth, 10, 100);
            } else {
                int presetWidth = truckPresets[truckSettings.currentPreset].width;
                int presetHeight = truckPresets[truckSettings.currentPreset].height;
                int presetDepth = truckPresets[truckSettings.currentPreset].depth;

                ImGui::InputInt("Ширина", &presetWidth, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Высота", &presetHeight, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Глубина", &presetDepth, 0, 0, ImGuiInputTextFlags_ReadOnly);
            }

            if (changed) {
                truckSettings.customWidth = std::max(300, std::min(3000, truckSettings.customWidth));
                truckSettings.customHeight = std::max(100, std::min(500, truckSettings.customHeight));
                truckSettings.customDepth = std::max(100, std::min(300, truckSettings.customDepth));
                updateTruckSize();
            }

            ImGui::PopItemWidth();
            ImGui::Separator();

            bool tentChanged = ImGui::Checkbox("Открыть тент", &truckSettings.tentOpen);
            if (tentChanged) {
                setTentOpen(truckSettings.tentOpen);
                if (currentScene) {
                    currentScene->setTentOpen(truckSettings.tentOpen);
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Renderer::renderTruckInfoPanel(const Scene& scene) {
    ImGui::Begin("Информация о грузовике", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    glm::vec3 currentSize = truckSettings.getCurrentSize();
    ImGui::Text("Текущий тип: %s", truckSettings.useCustom ? "Пользовательский" :
                truckPresets[truckSettings.currentPreset].name.c_str());
    ImGui::Text("Размеры: %.0f x %.0f x %.0f см", currentSize.x, currentSize.y, currentSize.z);
    ImGui::Text("Объем: %.2f м³", (currentSize.x * currentSize.y * currentSize.z) / 1000000.0f);
    ImGui::Text("Тент: %s", truckSettings.tentOpen ? "Открыт" : "Закрыт");

    ImGui::End();
}

void Renderer::renderPerformancePanel() {
    ImGui::Begin("Performance");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

glm::vec3 Renderer::TruckSettings::getCurrentSize() const {
    if (useCustom) {
        return glm::vec3(customWidth, customHeight, customDepth);
    } else {
        // Возвращаем размеры текущего пресета вместо значений по умолчанию
        if (currentPreset >= 0 && currentPreset < 6) {  // Проверяем границы
            const std::array<int, 3> presetSizes[6] = {
                {590, 239, 235},   // Малый грузовик
                {1203, 239, 235},  // Компактный грузовик
                {1340, 239, 235},  // Стандартный грузовик
                {1360, 260, 245},  // Средний грузовик
                {1360, 300, 245},  // Увеличенный грузовик
                {1650, 260, 245}   // Большой грузовик
            };
            const auto& preset = presetSizes[currentPreset];
            return glm::vec3(preset[0], preset[1], preset[2]);
        }
        return glm::vec3(1650, 260, 245); // Fallback
    }
}

void Renderer::updateTruckSize() {
    glm::vec3 size = truckSettings.getCurrentSize();
    std::cout << "Truck size updated: " << size.x << "x" << size.y << "x" << size.z << std::endl;
    if (currentScene) {
        currentScene->updateTruckSize(size.x, size.y, size.z);
        std::cout << "Scene truck size updated successfully" << std::endl;
    } else {
        std::cerr << "Warning: Scene not set in renderer, cannot update truck size" << std::endl;
    }
}

void Renderer::cleanupUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::setTentOpen(bool open) {
    truckSettings.tentOpen = open;
    updateTentAlpha();
}

bool Renderer::isTentOpen() const {
    return truckSettings.tentOpen;
}

void Renderer::updateTentAlpha() {
    if (truckSettings.tentOpen) {
        modelShader->setFloat("alpha", 0.0f); // Полностью прозрачный
    } else {
        modelShader->setFloat("alpha", truckSettings.tentAlpha); // Полупрозрачный
    }
    modelShader->setBool("useAlpha", true);
}
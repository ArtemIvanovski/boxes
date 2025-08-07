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

void Renderer::setScene(Scene *scene) {
    currentScene = scene;
}

void Renderer::initializeUI(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Try to load Russian font
    ImFont *font = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 18.0f, nullptr,
                                                io.Fonts->GetGlyphRangesCyrillic());
    if (font == nullptr) {
        io.Fonts->AddFontDefault();
        std::cout << "Warning: Could not load custom font, using default" << std::endl;
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Renderer::clear() {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(const Scene &scene, const Camera &camera) {
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

void Renderer::renderUI(const Scene &scene, GLFWwindow *window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderMainMenuBar(window);
    renderTruckInfoPanel(scene);
    renderPerformancePanel();

    // Рендерим панель коробок
    auto *boxManager = const_cast<Scene &>(scene).getBoxManager();
    if (boxManager) {
        boxManager->renderBoxPanel();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::renderMainMenuBar(GLFWwindow *window) {
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

        if (ImGui::BeginMenu("Вид")) {
            auto* boxManager = const_cast<Scene*>(currentScene)->getBoxManager();
            if (boxManager) {
                bool panelVisible = boxManager->isBoxPanelVisible();
                if (ImGui::MenuItem("Показать/скрыть панель коробок", "Tab", &panelVisible)) {
                    boxManager->toggleBoxPanel();
                }
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

void Renderer::renderTruckInfoPanel(const Scene &scene) {
    (void)scene; // Suppress unused parameter warning
    ImGuiIO &io = ImGui::GetIO();

    // Информация о фуре справа сверху (полупрозрачная)
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 350, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(340, 300), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.85f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));

    ImGui::Begin("##TruckInfo", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::Text("Информация о грузовике");
    ImGui::Separator();

    const char *presetNames[] = {
        "Малый (590x239x235)",
        "Компактный (1203x239x235)",
        "Стандартный (1340x239x235)",
        "Средний (1360x260x245)",
        "Увеличенный (1360x300x245)",
        "Большой (1650x260x245)"
    };

    if (ImGui::Combo("Тип грузовика", &truckSettings.currentPreset, presetNames, 6)) {
        updateTruckSize();
    }

    ImGui::Checkbox("Свои размеры", &truckSettings.useCustom);

    if (truckSettings.useCustom) {
        ImGui::PushItemWidth(80);
        ImGui::InputInt("Ширина", &truckSettings.customWidth, 10, 100);
        ImGui::SameLine();
        ImGui::InputInt("Высота", &truckSettings.customHeight, 10, 100);
        ImGui::SameLine();
        ImGui::InputInt("Глубина", &truckSettings.customDepth, 10, 100);
        ImGui::PopItemWidth();

        if (ImGui::Button("Применить размеры", ImVec2(-1, 25))) {
            updateTruckSize();
        }
    }

    ImGui::Separator();

    // Управление тентом
    ImGui::Text("Управление тентом:");
    bool tentOpen = truckSettings.tentOpen;
    if (ImGui::Checkbox("Тент открыт", &tentOpen)) {
        truckSettings.tentOpen = tentOpen;
        if (currentScene) {
            const_cast<Scene *>(currentScene)->setTentOpen(tentOpen);
        }
    }

    // Кнопки видов
    ImGui::Text("Виды камеры:");
    if (ImGui::Button("Сверху", ImVec2(80, 25))) {
        // Логика для вида сверху
    }
    ImGui::SameLine();
    if (ImGui::Button("Слева", ImVec2(80, 25))) {
        // Логика для вида слева
    }
    ImGui::SameLine();
    if (ImGui::Button("Справа", ImVec2(80, 25))) {
        // Логика для вида справа
    }

    if (ImGui::Button("Сброс камеры", ImVec2(-1, 25))) {
        // Логика сброса камеры
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

void Renderer::renderPerformancePanel() {
    ImGuiIO &io = ImGui::GetIO();

    // Простой FPS счетчик внизу справа
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 100, io.DisplaySize.y - 40), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(90, 30), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

    ImGui::Begin("##FPS", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::Text("%.0f FPS", io.Framerate);

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

glm::vec3 Renderer::TruckSettings::getCurrentSize() const {
    if (useCustom) {
        return glm::vec3(customWidth, customHeight, customDepth);
    } else {
        // Возвращаем размеры текущего пресета вместо значений по умолчанию
        if (currentPreset >= 0 && currentPreset < 6) {
            // Проверяем границы
            const std::array<int, 3> presetSizes[6] = {
                {590, 239, 235}, // Малый грузовик
                {1203, 239, 235}, // Компактный грузовик
                {1340, 239, 235}, // Стандартный грузовик
                {1360, 260, 245}, // Средний грузовик
                {1360, 300, 245}, // Увеличенный грузовик
                {1650, 260, 245} // Большой грузовик
            };
            const auto &preset = presetSizes[currentPreset];
            return glm::vec3(preset[0], preset[1], preset[2]);
        }
        return glm::vec3(1650, 260, 245); // Fallback
    }
}

void Renderer::updateTruckSize() {
    if (currentScene) {
        glm::vec3 size = truckSettings.getCurrentSize();
        currentScene->updateTruckSize(size.x, size.y, size.z);
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

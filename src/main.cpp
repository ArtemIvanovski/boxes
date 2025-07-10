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

// Предустановленные типы грузовиков
struct TruckPreset {
    std::string name;
    int width;
    int height;
    int depth;
};

std::vector<TruckPreset> truckPresets = {
    {"Малый грузовик", 1203, 239, 235},
    {"Средний грузовик", 1340, 239, 235},
    {"Большой грузовик", 1360, 260, 245},
    {"Увеличенный грузовик", 1360, 300, 245},
    {"Максимальный грузовик", 1650, 260, 245},
    {"Компактный грузовик", 590, 239, 235}
};

// Настройки грузовика
struct TruckSettings {
    int currentPreset = 2; // По умолчанию "Большой грузовик"
    int customWidth = 1650;
    int customHeight = 260;
    int customDepth = 245;
    bool useCustom = false;
    bool tentOpen = false;

    // Получить текущие размеры
    glm::vec3 getCurrentSize() const {
        if (useCustom) {
            return glm::vec3(customWidth, customHeight, customDepth);
        }
        return glm::vec3(truckPresets[currentPreset].width,
                        truckPresets[currentPreset].height,
                        truckPresets[currentPreset].depth);
    }
};

TruckSettings truckSettings;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void renderUI();
void setupOpenGLSettings();

void updateTruckSize() {
    glm::vec3 size = truckSettings.getCurrentSize();
    // Здесь можно добавить логику для обновления размеров 3D модели
    // Пока просто выводим информацию
    std::cout << "Truck size updated: " << size.x << "x" << size.y << "x" << size.z << std::endl;
}

// Заменить функцию setupImGuiStyle (найдите строки с ImLerp и замените)
void setupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Цвета в стиле Photoshop/Dark Theme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.28f, 0.28f, 0.28f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.35f, 0.35f, 0.35f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Стиль элементов
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 5.0f;
    style.TabRounding = 4.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;
}

// Упрощенная функция для красивых кнопок
bool IconButton(const char* label, const ImVec2& size = ImVec2(0, 0)) {
    return ImGui::Button(label, size);
}

// Простая кнопка переключения без использования внутренних функций
bool ToggleButton(const char* str_id, bool* v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float radius = height * 0.5f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        *v = !*v;
    }

    float t = *v ? 1.0f : 0.0f;

    // Простая анимация без внутренних функций
    static float anim_t = 0.0f;
    float target = *v ? 1.0f : 0.0f;
    anim_t += (target - anim_t) * 0.1f;

    ImU32 col_bg = *v ?
        ImGui::GetColorU32(ImVec4(0.2f, 0.7f, 0.2f, 1.0f)) :
        ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    if (ImGui::IsItemHovered()) {
        col_bg = *v ?
            ImGui::GetColorU32(ImVec4(0.3f, 0.8f, 0.3f, 1.0f)) :
            ImGui::GetColorU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    }

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + anim_t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));

    return *v;
}

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

    // ВАЖНО: Попробуем загрузить русский шрифт
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (font == nullptr) {
        // Если шрифт не загружается, используем стандартный
        io.Fonts->AddFontDefault();
        std::cout << "Warning: Could not load custom font, using default" << std::endl;
    }

    // Красивая темная тема (как в Photoshop)
    setupImGuiStyle();

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

    // Горячие клавиши для видов камеры
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1: // Вид сверху
                camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                camera->setRadius(20.0f);
                camera->setAlpha(glm::radians(90.0f));
                camera->setBeta(glm::radians(5.0f));
                break;
            case GLFW_KEY_2: // Вид слева
                camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                camera->setRadius(20.0f);
                camera->setAlpha(glm::radians(0.0f));
                camera->setBeta(glm::radians(90.0f));
                break;
            case GLFW_KEY_3: // Вид справа
                camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                camera->setRadius(20.0f);
                camera->setAlpha(glm::radians(180.0f));
                camera->setBeta(glm::radians(90.0f));
                break;
            case GLFW_KEY_4: // Изометрия
                camera->setTarget(glm::vec3(0.0f, 3.0f, 0.0f));
                camera->setRadius(20.0f);
                camera->setAlpha(glm::radians(45.0f));
                camera->setBeta(glm::radians(60.0f));
                break;
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

    // ===== ГЛАВНОЕ МЕНЮ =====
    if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("📁 Файл")) {
        if (ImGui::MenuItem("🆕 Новый проект", "Ctrl+N")) {
            // Реализация создания нового проекта
        }
        if (ImGui::MenuItem("📂 Открыть", "Ctrl+O")) {
            // Реализация открытия проекта
        }
        if (ImGui::MenuItem("💾 Сохранить", "Ctrl+S")) {
            // Реализация сохранения
        }
        ImGui::Separator();
        if (ImGui::MenuItem("🚪 Выход", "Alt+F4")) {
            glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("🚛 Грузовик")) {
        ImGui::Text("Тип прицепа:");
        ImGui::Separator();

        // Предустановленные размеры с иконками
        const char* icons[] = {"🚚", "🚛", "🚐", "🚌", "🚙", "🛻"};
        for (int i = 0; i < truckPresets.size(); i++) {
            bool selected = (truckSettings.currentPreset == i && !truckSettings.useCustom);
            std::string menuText = std::string(icons[i]) + " " + truckPresets[i].name;
            if (ImGui::MenuItem(menuText.c_str(), nullptr, selected)) {
                truckSettings.currentPreset = i;
                truckSettings.useCustom = false;
                updateTruckSize();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Размеры: %dx%dx%d см\nОбъем: %.2f м³",
                                truckPresets[i].width,
                                truckPresets[i].height,
                                truckPresets[i].depth,
                                (truckPresets[i].width * truckPresets[i].height * truckPresets[i].depth) / 1000000.0f);
            }
        }

        ImGui::Separator();
        bool customSelected = truckSettings.useCustom;
        if (ImGui::MenuItem("⚙️ Пользовательский", nullptr, customSelected)) {
            truckSettings.useCustom = true;
            updateTruckSize();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("🔧 Параметры")) {
        ImGui::Text("Размеры прицепа (см):");
        ImGui::PushItemWidth(100);

        bool changed = false;
        if (truckSettings.useCustom) {
            changed |= ImGui::InputInt("📏 Ширина##custom", &truckSettings.customWidth, 10, 100);
            changed |= ImGui::InputInt("📐 Высота##custom", &truckSettings.customHeight, 10, 100);
            changed |= ImGui::InputInt("📏 Глубина##custom", &truckSettings.customDepth, 10, 100);
        } else {
            int presetWidth = truckPresets[truckSettings.currentPreset].width;
            int presetHeight = truckPresets[truckSettings.currentPreset].height;
            int presetDepth = truckPresets[truckSettings.currentPreset].depth;

            ImGui::InputInt("📏 Ширина##preset", &presetWidth, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputInt("📐 Высота##preset", &presetHeight, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputInt("📏 Глубина##preset", &presetDepth, 0, 0, ImGuiInputTextFlags_ReadOnly);
        }

        if (changed) {
            // Ограничиваем размеры
            truckSettings.customWidth = std::max(300, std::min(3000, truckSettings.customWidth));
            truckSettings.customHeight = std::max(100, std::min(500, truckSettings.customHeight));
            truckSettings.customDepth = std::max(100, std::min(300, truckSettings.customDepth));
            updateTruckSize();
        }

        ImGui::PopItemWidth();
        ImGui::Separator();

        // Настройки тента с красивым переключателем
        ImGui::Text("🏠 Тент:");
        ImGui::SameLine();
        if (ToggleButton("TentToggle", &truckSettings.tentOpen)) {
            std::cout << "Tent " << (truckSettings.tentOpen ? "opened" : "closed") << std::endl;
        }
        ImGui::SameLine();
        ImGui::Text(truckSettings.tentOpen ? "Открыт" : "Закрыт");

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("👁️ Вид")) {
        if (ImGui::MenuItem("⬆️ Сверху", "1")) {
            camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
            camera->setRadius(20.0f);
            camera->setAlpha(glm::radians(90.0f));
            camera->setBeta(glm::radians(5.0f));
        }
        if (ImGui::MenuItem("⬅️ Слева", "2")) {
            camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
            camera->setRadius(20.0f);
            camera->setAlpha(glm::radians(0.0f));
            camera->setBeta(glm::radians(90.0f));
        }
        if (ImGui::MenuItem("➡️ Справа", "3")) {
            camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
            camera->setRadius(20.0f);
            camera->setAlpha(glm::radians(180.0f));
            camera->setBeta(glm::radians(90.0f));
        }
        if (ImGui::MenuItem("🔄 Изометрия", "4")) {
            camera->setTarget(glm::vec3(0.0f, 3.0f, 0.0f));
            camera->setRadius(20.0f);
            camera->setAlpha(glm::radians(45.0f));
            camera->setBeta(glm::radians(60.0f));
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("❓ Справка")) {
        if (ImGui::MenuItem("ℹ️ О программе")) {
            // Показать информацию о программе
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

    // ===== ИНФОРМАЦИОННАЯ ПАНЕЛЬ =====
    ImGui::Begin("Информация о грузовике", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    glm::vec3 currentSize = truckSettings.getCurrentSize();
    ImGui::Text("Текущий тип: %s", truckSettings.useCustom ? "Пользовательский" :
                truckPresets[truckSettings.currentPreset].name.c_str());
    ImGui::Text("Размеры: %.0f x %.0f x %.0f см", currentSize.x, currentSize.y, currentSize.z);
    ImGui::Text("Объем: %.2f м³", (currentSize.x * currentSize.y * currentSize.z) / 1000000.0f);
    ImGui::Text("Тент: %s", truckSettings.tentOpen ? "Открыт" : "Закрыт");

    ImGui::End();

    // ===== ОСТАЛЬНЫЕ ОКНА (можно оставить как есть) =====

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
    ImGui::Text("M: Toggle material override");
    ImGui::Text("1-4: Camera presets");
    ImGui::Text("ESC: Exit");

    ImGui::End();

    // Material Settings Window (оставьте как есть)
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
#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../graphics/Shader.h"
#include "../graphics/Camera.h"
#include "../scene/Scene.h"

class Renderer {
private:
    std::unique_ptr<Shader> modelShader;

    // UI
    void renderMainMenuBar(GLFWwindow* window);
    void renderTruckInfoPanel(const Scene& scene);
    void renderPerformancePanel();

    // Settings
    struct TruckSettings {
        int currentPreset = 2;
        int customWidth = 1650;
        int customHeight = 260;
        int customDepth = 245;
        bool useCustom = false;
        bool tentOpen = false;

        glm::vec3 getCurrentSize() const;
    } truckSettings;

    struct TruckPreset {
        std::string name;
        int width, height, depth;
    };

    std::vector<TruckPreset> truckPresets;

    void updateTruckSize();

public:
    Renderer();
    ~Renderer();

    void initializeUI(GLFWwindow* window);
    void clear();
    void render(const Scene& scene, const Camera& camera);
    void renderUI(const Scene& scene, GLFWwindow* window);
    void cleanupUI();
};

#endif // RENDERER_H
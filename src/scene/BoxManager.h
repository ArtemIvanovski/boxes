#ifndef BOXMANAGER_H
#define BOXMANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "../graphics/Mesh.h"
#include "../graphics/Material.h"

struct BoxData {
    std::string name;
    float width, height, depth;  // в см
    float weight;
    int quantity;
    glm::vec3 color;
    bool isSelected = false;
    bool isOnScene = false;
    bool hasMarkings = true;
    std::string markingText;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    
    BoxData(const std::string& n, float w, float h, float d, float wt, int qty, const glm::vec3& c)
        : name(n), width(w), height(h), depth(d), weight(wt), quantity(qty), color(c) {}
};

class BoxManager {
private:
    std::vector<BoxData> boxes;
    std::vector<std::unique_ptr<Mesh>> boxMeshes;
    int selectedBoxIndex = -1;
    bool isDragging = false;
    glm::vec3 dragStartPos;
    
    // UI dragging state
    bool draggingFromUI = false;
    int draggingBoxIndex = -1;
    glm::vec3 uiDragStartPos;
    
    // UI state
    bool showBoxPanel = true;
    std::string newBoxName = "PO#0000";
    float newBoxWidth = 100.0f;
    float newBoxHeight = 100.0f;
    float newBoxDepth = 100.0f;
    float newBoxWeight = 1.0f;
    int newBoxQuantity = 1;
    glm::vec3 newBoxColor = glm::vec3(0.7f, 0.7f, 0.7f);

public:
    BoxManager();
    ~BoxManager() = default;

    // Box management
    void addBox(const std::string& name, float width, float height, float depth, float weight, int quantity, const glm::vec3& color);
    void removeBox(int index);
    void clearBoxes();
    
    // Selection and interaction
    void selectBox(int index);
    void deselectBox();
    int getSelectedBoxIndex() const { return selectedBoxIndex; }
    bool isBoxSelected() const { return selectedBoxIndex >= 0; }
    
    // Dragging
    void startDragging(const glm::vec3& startPos);
    void updateDragging(const glm::vec3& currentPos);
    void stopDragging();
    bool isDraggingBox() const { return isDragging; }
    
    // Rendering
    void renderBoxes(const Shader& shader);
    void renderBoxPanel();
    
    // Getters
    const std::vector<BoxData>& getBoxes() const { return boxes; }
    const BoxData* getSelectedBox() const;
    BoxData* getSelectedBox();
    
    // UI
    void toggleBoxPanel() { showBoxPanel = !showBoxPanel; }
    bool isBoxPanelVisible() const { return showBoxPanel; }
    
    // Input handling
    void handleMouseInput(double xpos, double ypos, bool leftPressed, bool rightPressed);
    void handleKeyInput(int key, int action);
    
    // Box dragging from UI to scene
    void startBoxDrag(int boxIndex);
    void updateBoxDrag(double xpos, double ypos);
    void endBoxDrag();
    bool isDraggingFromUI() const { return draggingFromUI; }
    int getDraggingBoxIndex() const { return draggingBoxIndex; }
};

#endif // BOXMANAGER_H

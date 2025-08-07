#include "BoxManager.h"
#include "../graphics/Primitives.h"
#include <imgui.h>
#include <iostream>
#include <algorithm>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

BoxManager::BoxManager() {
    // Добавляем тестовые коробки (как в BabylonJS)
    addBox("PO#2457", 197, 179, 197, 2.0f, 1, glm::vec3(0.8f, 0.2f, 0.2f));
    addBox("PO#2458", 197, 179, 197, 2.0f, 1, glm::vec3(0.2f, 0.8f, 0.2f));
    addBox("PO#2459", 197, 179, 197, 3.0f, 1, glm::vec3(0.2f, 0.2f, 0.8f));
    addBox("PO#2460", 122, 122, 197, 3.0f, 1, glm::vec3(0.8f, 0.8f, 0.2f));
    addBox("PO#2461", 122, 122, 197, 3.0f, 1, glm::vec3(0.8f, 0.2f, 0.8f));
}

void BoxManager::addBox(const std::string& name, float width, float height, float depth, float weight, int quantity, const glm::vec3& color) {
    boxes.emplace_back(name, width, height, depth, weight, quantity, color);
    
    // Создаем меш для коробки (как в BabylonJS)
    Material boxMaterial;
    boxMaterial.diffuse = color; // emissiveColor как в BabylonJS
    boxMaterial.ambient = color * 0.2f;
    boxMaterial.specular = glm::vec3(0.2f, 0.2f, 0.2f); // specularColor как в BabylonJS
    boxMaterial.shininess = 32.0f;
    
    // Конвертируем см в метры
    float w = width / 100.0f;
    float h = height / 100.0f;
    float d = depth / 100.0f;

    boxMeshes.push_back(Primitives::createBox(w, h, d, boxMaterial));
    boxes.back().markingText = name + "\n" + std::to_string(static_cast<int>(width)) + "x" +
                          std::to_string(static_cast<int>(height)) + "x" +
                          std::to_string(static_cast<int>(depth));
}

void BoxManager::removeBox(int index) {
    if (index >= 0 && index < static_cast<int>(boxes.size())) {
        boxes.erase(boxes.begin() + index);
        boxMeshes.erase(boxMeshes.begin() + index);
        
        if (selectedBoxIndex == index) {
            deselectBox();
        } else if (selectedBoxIndex > index) {
            selectedBoxIndex--;
        }
    }
}

void BoxManager::clearBoxes() {
    boxes.clear();
    boxMeshes.clear();
    deselectBox();
}

void BoxManager::selectBox(int index) {
    if (index >= 0 && index < static_cast<int>(boxes.size())) {
        // Deselect previous
        if (selectedBoxIndex >= 0 && selectedBoxIndex < static_cast<int>(boxes.size())) {
            boxes[selectedBoxIndex].isSelected = false;
        }
        
        selectedBoxIndex = index;
        boxes[index].isSelected = true;
    }
}

void BoxManager::deselectBox() {
    if (selectedBoxIndex >= 0 && selectedBoxIndex < static_cast<int>(boxes.size())) {
        boxes[selectedBoxIndex].isSelected = false;
    }
    selectedBoxIndex = -1;
}

const BoxData* BoxManager::getSelectedBox() const {
    if (selectedBoxIndex >= 0 && selectedBoxIndex < static_cast<int>(boxes.size())) {
        return &boxes[selectedBoxIndex];
    }
    return nullptr;
}

BoxData* BoxManager::getSelectedBox() {
    if (selectedBoxIndex >= 0 && selectedBoxIndex < static_cast<int>(boxes.size())) {
        return &boxes[selectedBoxIndex];
    }
    return nullptr;
}

void BoxManager::startDragging(const glm::vec3& startPos) {
    if (selectedBoxIndex >= 0) {
        isDragging = true;
        dragStartPos = startPos;
        boxes[selectedBoxIndex].isOnScene = true;
    }
}

void BoxManager::updateDragging(const glm::vec3& currentPos) {
    if (isDragging && selectedBoxIndex >= 0) {
        boxes[selectedBoxIndex].position = currentPos;
    }
}

void BoxManager::stopDragging() {
    isDragging = false;
}

void BoxManager::startBoxDrag(int boxIndex) {
    if (boxIndex >= 0 && boxIndex < static_cast<int>(boxes.size())) {
        draggingFromUI = true;
        draggingBoxIndex = boxIndex;
        boxes[boxIndex].isOnScene = true;
        // Set initial position above ground
        boxes[boxIndex].position = glm::vec3(0.0f, boxes[boxIndex].height / 200.0f, 0.0f);
    }
}

void BoxManager::updateBoxDrag(double xpos, double ypos) {
    if (draggingFromUI && draggingBoxIndex >= 0) {
        // Convert screen coordinates to world coordinates
        // This is a simplified conversion - in a real implementation,
        // you'd need proper ray casting from camera
        float worldX = (xpos / 800.0f - 0.5f) * 20.0f; // Assuming 800px width
        float worldZ = (ypos / 600.0f - 0.5f) * 20.0f; // Assuming 600px height
        
        boxes[draggingBoxIndex].position.x = worldX;
        boxes[draggingBoxIndex].position.z = worldZ;
        boxes[draggingBoxIndex].position.y = boxes[draggingBoxIndex].height / 200.0f;
    }
}

void BoxManager::endBoxDrag() {
    draggingFromUI = false;
    draggingBoxIndex = -1;
}

void BoxManager::renderBoxes(const Shader& shader) {
    for (size_t i = 0; i < boxes.size(); ++i) {
        if (boxes[i].isOnScene) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, boxes[i].position);
            model = glm::rotate(model, boxes[i].rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, boxes[i].rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, boxes[i].rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            
            shader.setMat4("model", model);
            shader.setBool("use_material_override", false);
            
            if (i < boxMeshes.size()) {
                boxMeshes[i]->draw(shader);
            }
        }
        if (boxes[i].hasMarkings) {
            // Здесь можно использовать ImGui для отображения текста в 3D позиции
            // или создать текстурированные плоскости с текстом
        }
    }
}

void BoxManager::renderBoxPanel() {
    if (!showBoxPanel) return;
    
    ImGuiIO& io = ImGui::GetIO();

    // Sidebar справа (как в HTML)
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(310, io.DisplaySize.y - 120), ImGuiCond_Always);

    ImGui::Begin("##BoxSidebar", &showBoxPanel,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::Text("Управление коробками");
    ImGui::Separator();

    // Панель добавления новой коробки
    ImGui::Text("Добавить коробку:");

    char nameBuffer[256];
    strcpy_s(nameBuffer, newBoxName.c_str());
    if (ImGui::InputText("Название", nameBuffer, sizeof(nameBuffer))) {
        newBoxName = nameBuffer;
    }

    ImGui::PushItemWidth(70);
    ImGui::InputFloat("Ширина", &newBoxWidth, 1.0f, 10.0f, "%.0f");
    ImGui::SameLine();
    ImGui::InputFloat("Высота", &newBoxHeight, 1.0f, 10.0f, "%.0f");
    ImGui::SameLine();
    ImGui::InputFloat("Глубина", &newBoxDepth, 1.0f, 10.0f, "%.0f");
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(70);
    ImGui::InputFloat("Вес", &newBoxWeight, 0.1f, 1.0f, "%.1f");
    ImGui::SameLine();
    ImGui::InputInt("Кол-во", &newBoxQuantity, 1, 10);
    ImGui::PopItemWidth();

    float color[3] = {newBoxColor.x, newBoxColor.y, newBoxColor.z};
    if (ImGui::ColorEdit3("Цвет", color)) {
        newBoxColor = glm::vec3(color[0], color[1], color[2]);
    }

    if (ImGui::Button("Добавить", ImVec2(-1, 30))) {
        addBox(newBoxName, newBoxWidth, newBoxHeight, newBoxDepth, newBoxWeight, newBoxQuantity, newBoxColor);

        newBoxName = "PO#" + std::to_string(boxes.size() + 2457);
        newBoxWidth = 100.0f;
        newBoxHeight = 100.0f;
        newBoxDepth = 100.0f;
        newBoxWeight = 1.0f;
        newBoxQuantity = 1;
        newBoxColor = glm::vec3(0.7f, 0.7f, 0.7f);
    }

    ImGui::Separator();

    // Список коробок с 2D превью
    ImGui::Text("Коробки (%zu):", boxes.size());
    ImGui::BeginChild("BoxGrid", ImVec2(0, -80), true);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    float cellSize = 60.0f;
    int columns = 4;

    for (size_t i = 0; i < boxes.size(); ++i) {
        const auto& box = boxes[i];

        int row = i / columns;
        int col = i % columns;

        ImVec2 cellPos = ImVec2(canvasPos.x + col * (cellSize + 5), canvasPos.y + row * (cellSize + 5));
        ImVec2 cellEnd = ImVec2(cellPos.x + cellSize, cellPos.y + cellSize);

        // Рисуем 2D представление коробки
        ImU32 boxColor = IM_COL32(box.color.x * 255, box.color.y * 255, box.color.z * 255, 255);
        ImU32 borderColor = (selectedBoxIndex == i) ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 0, 0, 255);

        drawList->AddRectFilled(cellPos, cellEnd, boxColor);
        drawList->AddRect(cellPos, cellEnd, borderColor, 0.0f, 0, 2.0f);

        // Текст на коробке
        std::string label = std::to_string(static_cast<int>(box.width)) + "x" +
                           std::to_string(static_cast<int>(box.height));
        ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        ImVec2 textPos = ImVec2(cellPos.x + (cellSize - textSize.x) * 0.5f,
                               cellPos.y + (cellSize - textSize.y) * 0.5f);
        drawList->AddText(textPos, IM_COL32(0, 0, 0, 255), label.c_str());

        // Обработка кликов
        if (ImGui::IsMouseHoveringRect(cellPos, cellEnd)) {
            if (ImGui::IsMouseClicked(0)) {
                selectBox(static_cast<int>(i));
            }

            // Tooltip
            ImGui::BeginTooltip();
            ImGui::Text("Название: %s", box.name.c_str());
            ImGui::Text("Размеры: %.0fx%.0fx%.0f см", box.width, box.height, box.depth);
            ImGui::Text("Вес: %.1f кг", box.weight);
            ImGui::Text("На сцене: %s", box.isOnScene ? "Да" : "Нет");
            ImGui::EndTooltip();
        }
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((boxes.size() / columns) + 1) * (cellSize + 5));
    ImGui::EndChild();

    // Кнопки управления
    if (selectedBoxIndex >= 0) {
        if (ImGui::Button("Удалить", ImVec2(90, 25))) {
            removeBox(selectedBoxIndex);
        }
        ImGui::SameLine();
        if (ImGui::Button("Очистить все", ImVec2(90, 25))) {
            clearBoxes();
        }
        ImGui::SameLine();
        auto* selected = getSelectedBox();
        if (selected && !selected->isOnScene) {
            if (ImGui::Button("На сцену", ImVec2(90, 25))) {
                selected->isOnScene = true;
                selected->position = glm::vec3(0.0f, selected->height / 200.0f, 0.0f);
            }
        }
    }

    ImGui::End();
}

void BoxManager::handleMouseInput(double xpos, double ypos, bool leftPressed, bool rightPressed) {
    (void)rightPressed; // Suppress unused parameter warning
    // Handle box dragging from UI to scene
    if (leftPressed && selectedBoxIndex >= 0 && !draggingFromUI) {
        startBoxDrag(selectedBoxIndex);
    }
    
    if (draggingFromUI) {
        updateBoxDrag(xpos, ypos);
        
        if (!leftPressed) {
            endBoxDrag();
        }
    }
}

void BoxManager::handleKeyInput(int key, int action) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_DELETE:
            case GLFW_KEY_BACKSPACE:
                if (selectedBoxIndex >= 0) {
                    removeBox(selectedBoxIndex);
                }
                break;
            case GLFW_KEY_R:
                if (selectedBoxIndex >= 0) {
                    auto* box = getSelectedBox();
                    if (box) {
                        box->rotation.y += glm::radians(90.0f);
                        if (box->rotation.y >= glm::radians(360.0f)) {
                            box->rotation.y = 0.0f;
                        }
                    }
                }
            break;
            case GLFW_KEY_T:
                if (selectedBoxIndex >= 0) {
                    auto* box = getSelectedBox();
                    if (box) {
                        box->rotation.x += glm::radians(90.0f);
                        if (box->rotation.x >= glm::radians(360.0f)) {
                            box->rotation.x = 0.0f;
                        }
                    }
                }
            break;
        }
    }
}

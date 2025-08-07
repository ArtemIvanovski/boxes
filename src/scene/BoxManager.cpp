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
    }
}

void BoxManager::renderBoxPanel() {
    if (!showBoxPanel) return;
    
    // Фиксированная панель слева (как в BabylonJS)
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Управление коробками", &showBoxPanel, 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    // Панель добавления новой коробки
    ImGui::Text("Добавить новую коробку:");
    ImGui::Separator();
    
    char nameBuffer[256];
    strcpy_s(nameBuffer, newBoxName.c_str());
    if (ImGui::InputText("Название", nameBuffer, sizeof(nameBuffer))) {
        newBoxName = nameBuffer;
    }
    ImGui::InputFloat("Ширина (см)", &newBoxWidth, 1.0f, 10.0f, "%.0f");
    ImGui::InputFloat("Высота (см)", &newBoxHeight, 1.0f, 10.0f, "%.0f");
    ImGui::InputFloat("Глубина (см)", &newBoxDepth, 1.0f, 10.0f, "%.0f");
    ImGui::InputFloat("Вес (кг)", &newBoxWeight, 0.1f, 1.0f, "%.1f");
    ImGui::InputInt("Количество", &newBoxQuantity, 1, 10);
    
    // Color picker (как в BabylonJS)
    float color[3] = {newBoxColor.x, newBoxColor.y, newBoxColor.z};
    if (ImGui::ColorEdit3("Цвет", color)) {
        newBoxColor = glm::vec3(color[0], color[1], color[2]);
    }
    
    if (ImGui::Button("Добавить коробку")) {
        addBox(newBoxName, newBoxWidth, newBoxHeight, newBoxDepth, newBoxWeight, newBoxQuantity, newBoxColor);
        
        // Reset form
        newBoxName = "PO#" + std::to_string(boxes.size() + 2457);
        newBoxWidth = 100.0f;
        newBoxHeight = 100.0f;
        newBoxDepth = 100.0f;
        newBoxWeight = 1.0f;
        newBoxQuantity = 1;
        newBoxColor = glm::vec3(0.7f, 0.7f, 0.7f);
    }
    
    ImGui::Separator();
    
    // Список существующих коробок (как в BabylonJS)
    ImGui::Text("Существующие коробки:");
    ImGui::Text("(Перетащите коробки на сцену левой кнопкой мыши)");
    ImGui::BeginChild("BoxList", ImVec2(0, 200), true);
    
    for (size_t i = 0; i < boxes.size(); ++i) {
        const auto& box = boxes[i];
        
        // Отображаем как прямоугольники с цветом (как в BabylonJS)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(box.color.x, box.color.y, box.color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(box.color.x * 1.2f, box.color.y * 1.2f, box.color.z * 1.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(box.color.x * 0.8f, box.color.y * 0.8f, box.color.z * 0.8f, 1.0f));
        
        std::string label = box.name + " (" + std::to_string(static_cast<int>(box.width)) + "x" + 
                           std::to_string(static_cast<int>(box.height)) + "x" + 
                           std::to_string(static_cast<int>(box.depth)) + " см)";
        
        if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetWindowWidth() - 20, 30))) {
            selectBox(static_cast<int>(i));
        }
        
        ImGui::PopStyleColor(3);
        
        // Tooltip с детальной информацией (как в BabylonJS)
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Название: %s", box.name.c_str());
            ImGui::Text("Размеры: %.0f x %.0f x %.0f см", box.width, box.height, box.depth);
            ImGui::Text("Вес: %.1f кг", box.weight);
            ImGui::Text("Количество: %d", box.quantity);
            ImGui::Text("На сцене: %s", box.isOnScene ? "Да" : "Нет");
            ImGui::EndTooltip();
        }
    }
    
    ImGui::EndChild();
    
    // Кнопки управления
    if (selectedBoxIndex >= 0) {
        ImGui::Separator();
        
        if (ImGui::Button("Удалить выбранную")) {
            removeBox(selectedBoxIndex);
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Очистить все")) {
            clearBoxes();
        }
        
        ImGui::SameLine();
        
        auto* selected = getSelectedBox();
        if (selected && !selected->isOnScene) {
            if (ImGui::Button("Разместить на сцене")) {
                selected->isOnScene = true;
                selected->position = glm::vec3(0.0f, selected->height / 200.0f, 0.0f); // Над полом
            }
        }
    }
    
    ImGui::End();
}

void BoxManager::handleMouseInput(double xpos, double ypos, bool leftPressed, bool rightPressed) {
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
                    }
                }
                break;
        }
    }
}

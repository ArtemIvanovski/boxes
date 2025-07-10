//
// Created by Pasha on 09.07.2025.
//

#ifndef MODEL_H
#define MODEL_H

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Shader.h"

class Model {
private:
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::string directory;
    std::vector<Texture> texturesLoaded;

    // Optimization: Cache for frequently used data
    mutable bool boundingBoxCached = false;
    mutable glm::vec3 cachedMinBounds;
    mutable glm::vec3 cachedMaxBounds;

public:
    Model(const std::string& path);
    ~Model() = default;

    void draw(const Shader& shader) const;
    void drawInstanced(const Shader& shader, unsigned int amount) const;

    // Bounding box calculations
    glm::vec3 getMinBounds() const;
    glm::vec3 getMaxBounds() const;
    glm::vec3 getCenter() const;
    glm::vec3 getSize() const;

    // Optimization methods
    void optimizeMeshes();
    size_t getTriangleCount() const;
    size_t getVertexCount() const;

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    unsigned int textureFromFile(const char* path, const std::string& modelDirectory);
    void calculateBoundingBox() const;
};

// Model.cpp implementation
#include "Model.h"
#include <glad/glad.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::draw(const Shader& shader) const {
    for (const auto& mesh : meshes) {
        mesh->draw(shader);
    }
}

void Model::drawInstanced(const Shader& shader, unsigned int amount) const {
    for (const auto& mesh : meshes) {
        mesh->drawInstanced(shader, amount);
    }
}

glm::vec3 Model::getMinBounds() const {
    if (!boundingBoxCached) calculateBoundingBox();
    return cachedMinBounds;
}

glm::vec3 Model::getMaxBounds() const {
    if (!boundingBoxCached) calculateBoundingBox();
    return cachedMaxBounds;
}

glm::vec3 Model::getCenter() const {
    return (getMinBounds() + getMaxBounds()) * 0.5f;
}

glm::vec3 Model::getSize() const {
    return getMaxBounds() - getMinBounds();
}

size_t Model::getTriangleCount() const {
    size_t count = 0;
    for (const auto& mesh : meshes) {
        count += mesh->getIndices().size() / 3;
    }
    return count;
}

size_t Model::getVertexCount() const {
    size_t count = 0;
    for (const auto& mesh : meshes) {
        count += mesh->getVertices().size();
    }
    return count;
}

void Model::optimizeMeshes() {
    // Remove duplicate vertices, optimize index buffers
    for (auto& mesh : meshes) {
        mesh->optimize();
    }
}

void Model::loadModel(const std::string& path) {
    std::cout << "Attempting to load model: " << path << std::endl;

    // Проверяем существование файлов
    std::ifstream objFile(path);
    if (!objFile.good()) {
        std::cerr << "OBJ file does not exist: " << path << std::endl;
        throw std::runtime_error("OBJ file not found: " + path);
    }
    objFile.close();

    // Проверяем существование .mtl файла
    std::string mtlPath = path.substr(0, path.find_last_of('.')) + ".mtl";
    std::ifstream mtlFile(mtlPath);
    if (mtlFile.good()) {
        std::cout << "Found corresponding MTL file: " << mtlPath << std::endl;
        mtlFile.close();
    } else {
        std::cout << "Warning: No MTL file found at: " << mtlPath << std::endl;
    }

    Assimp::Importer importer;

    // Более конкретные флаги для обработки .obj/.mtl файлов
    unsigned int flags = aiProcess_Triangulate |           // Конвертируем все полигоны в треугольники
                        aiProcess_FlipUVs |                // Переворачиваем UV координаты
                        aiProcess_GenNormals |             // Генерируем нормали если их нет
                        aiProcess_GenSmoothNormals |       // Генерируем сглаженные нормали
                        aiProcess_OptimizeMeshes |         // Оптимизируем меши
                        aiProcess_OptimizeGraph |          // Оптимизируем граф сцены
                        aiProcess_JoinIdenticalVertices |  // Объединяем одинаковые вершины
                        aiProcess_ImproveCacheLocality |   // Улучшаем cache locality
                        aiProcess_RemoveRedundantMaterials | // Удаляем дублирующиеся материалы
                        aiProcess_FixInfacingNormals |     // Исправляем направление нормалей
                        aiProcess_CalcTangentSpace |       // Вычисляем тангенс пространство
                        aiProcess_ValidateDataStructure |  // Проверяем структуру данных
                        aiProcess_SortByPType;             // Сортируем по типу примитива

    std::cout << "Loading with flags: " << std::hex << flags << std::dec << std::endl;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        std::cerr << "Failed to load model at path: " << path << std::endl;
        throw std::runtime_error("Failed to load model: " + path);
    }

    std::cout << "Model loaded successfully!" << std::endl;
    std::cout << "Scene info:" << std::endl;
    std::cout << "  Meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "  Materials: " << scene->mNumMaterials << std::endl;
    std::cout << "  Textures: " << scene->mNumTextures << std::endl;
    std::cout << "  Animations: " << scene->mNumAnimations << std::endl;

    // Выводим информацию о материалах для отладки
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        aiString name;
        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            std::cout << "  Material " << i << ": " << name.C_Str() << std::endl;
        }
    }

    directory = path.substr(0, path.find_last_of('/'));
    std::cout << "Model directory: " << directory << std::endl;

    processNode(scene->mRootNode, scene);

    // Post-loading optimizations
    optimizeMeshes();

    std::cout << "Model processing completed!" << std::endl;
    std::cout << "Final statistics:" << std::endl;
    std::cout << "  Total meshes created: " << meshes.size() << std::endl;
    std::cout << "  Total triangles: " << getTriangleCount() << std::endl;
    std::cout << "  Total vertices: " << getVertexCount() << std::endl;
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Process all child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Material material; // Создаем материал

    // Reserve memory for better performance
    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Position
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normals
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials - УЛУЧШЕННАЯ ЗАГРУЗКА МАТЕРИАЛОВ
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        // Получаем имя материала для отладки
        aiString materialName;
        if (mat->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS) {
            std::cout << "Loading material: " << materialName.C_Str() << std::endl;
        }

        // Загружаем цвета материала
        aiColor3D color;

        // Ambient color
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            material.ambient = glm::vec3(color.r, color.g, color.b);
            std::cout << "  Ambient: (" << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;
        } else {
            // Устанавливаем разумные значения по умолчанию
            material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        }

        // Diffuse color (основной цвет)
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            material.diffuse = glm::vec3(color.r, color.g, color.b);
            std::cout << "  Diffuse: (" << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;

            // Проверяем, не слишком ли темный материал
            float brightness = (color.r + color.g + color.b) / 3.0f;
            if (brightness < 0.05f) {
                std::cout << "  Warning: Material is very dark, using default gray" << std::endl;
                material.diffuse = glm::vec3(0.6f, 0.6f, 0.6f);
            }
        } else {
            // Если диффузный цвет не найден, используем светло-серый
            std::cout << "  No diffuse color found, using default" << std::endl;
            material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        }

        // Specular color
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            material.specular = glm::vec3(color.r, color.g, color.b);
            std::cout << "  Specular: (" << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;
        } else {
            material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        }

        // Shininess
        float shininess;
        if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            material.shininess = shininess;
            std::cout << "  Shininess: " << shininess << std::endl;
        } else {
            material.shininess = 32.0f;
        }

        // Дополнительные свойства материала для отладки
        float opacity;
        if (mat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
            std::cout << "  Opacity: " << opacity << std::endl;
        }

        // Metallic/Roughness для PBR материалов (если есть)
        float metallic, roughness;
        if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
            std::cout << "  Metallic: " << metallic << std::endl;
        }
        if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            std::cout << "  Roughness: " << roughness << std::endl;
        }

        // Загружаем текстуры
        std::vector<Texture> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        if (!diffuseMaps.empty()) {
            std::cout << "  Found " << diffuseMaps.size() << " diffuse texture(s)" << std::endl;
        }

        std::vector<Texture> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        if (!specularMaps.empty()) {
            std::cout << "  Found " << specularMaps.size() << " specular texture(s)" << std::endl;
        }

        // Также попробуем загрузить normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        if (!normalMaps.empty()) {
            std::cout << "  Found " << normalMaps.size() << " normal texture(s)" << std::endl;
        }

        std::cout << "  Material loaded successfully!" << std::endl;
    } else {
        std::cout << "No material found for mesh, using default material" << std::endl;
        // Устанавливаем значения по умолчанию
        material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        material.shininess = 32.0f;
    }

    // Создаем mesh с материалом
    return std::make_unique<Mesh>(vertices, indices, textures, material);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        // Check if texture was loaded before
        bool skip = false;
        for (const auto& texture : texturesLoaded) {
            if (std::strcmp(texture.path.data(), str.C_Str()) == 0) {
                textures.push_back(texture);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            texture.id = textureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture);
        }
    }

    return textures;
}

unsigned int Model::textureFromFile(const char* path, const std::string& modelDirectory) {
    std::string filename = std::string(path);
    filename = modelDirectory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Standard texture parameters (removed anisotropic filtering to fix compilation)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void Model::calculateBoundingBox() const {
    if (meshes.empty()) return;

    cachedMinBounds = glm::vec3(FLT_MAX);
    cachedMaxBounds = glm::vec3(-FLT_MAX);

    for (const auto& mesh : meshes) {
        const auto& vertices = mesh->getVertices();
        for (const auto& vertex : vertices) {
            cachedMinBounds = glm::min(cachedMinBounds, vertex.position);
            cachedMaxBounds = glm::max(cachedMaxBounds, vertex.position);
        }
    }

    boundingBoxCached = true;
}

#endif //MODEL_H
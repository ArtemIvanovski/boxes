#include "Model.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>

// ВАЖНО: STB_IMAGE_IMPLEMENTATION должен быть ТОЛЬКО в одном .cpp файле
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

    // Проверяем существование файла
    std::ifstream file(path);
    if (!file.good()) {
        std::cerr << "Model file does not exist: " << path << std::endl;
        throw std::runtime_error("Model file not found: " + path);
    }
    file.close();

    Assimp::Importer importer;

    // Улучшенные флаги для обработки моделей
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
                        aiProcess_CalcTangentSpace |       // Вычисляем tangent space
                        aiProcess_ValidateDataStructure;   // Валидируем структуру данных

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        throw std::runtime_error("Failed to load model: " + path);
    }

    std::cout << "Model loaded successfully: " << path << std::endl;
    std::cout << "Meshes count: " << scene->mNumMeshes << std::endl;
    std::cout << "Materials count: " << scene->mNumMaterials << std::endl;

    // Сохраняем путь к директории для загрузки текстур
    directory = path.substr(0, path.find_last_of('/'));

    // Обрабатываем все узлы рекурсивно
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Обрабатываем все меши узла
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Рекурсивно обрабатываем дочерние узлы
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Обрабатываем вершины
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Позиция
        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Нормали
        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default normal
        }

        // Текстурные координаты
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Обрабатываем индексы
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Обрабатываем материалы
    Material material;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        
        // Загружаем диффузные текстуры
        std::vector<Texture> diffuseMaps = loadMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // Загружаем specular текстуры
        std::vector<Texture> specularMaps = loadMaterialTextures(aiMat, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // Загружаем нормальные карты
        std::vector<Texture> normalMaps = loadMaterialTextures(aiMat, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // Загружаем высотные карты
        std::vector<Texture> heightMaps = loadMaterialTextures(aiMat, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // Устанавливаем материал
        aiColor3D color(0.0f, 0.0f, 0.0f);
        float shininess = 32.0f;

        // Диффузный цвет
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            material.diffuse = glm::vec3(color.r, color.g, color.b);
        } else {
            material.diffuse = glm::vec3(0.7f, 0.7f, 0.7f); // Default gray
        }

        // Ambient цвет
        if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            material.ambient = glm::vec3(color.r, color.g, color.b);
        } else {
            material.ambient = glm::vec3(0.2f, 0.2f, 0.2f); // Default ambient
        }

        // Specular цвет
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            material.specular = glm::vec3(color.r, color.g, color.b);
        } else {
            material.specular = glm::vec3(0.5f, 0.5f, 0.5f); // Default specular
        }

        // Shininess
        if (aiMat->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS) {
            shininess = 32.0f; // Default shininess
        }
        material.shininess = shininess;

        // Если материал слишком темный, делаем его светлее
        if (glm::length(material.diffuse) < 0.1f) {
            material.diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
        }
    } else {
        // Default material
        material.diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
        material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        material.shininess = 32.0f;
    }

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
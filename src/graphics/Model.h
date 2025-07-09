//
// Created by Pasha on 09.07.2025.
//

#ifndef MODEL_H
#define MODEL_H



#pragma once

#include <string>
#include <vector>
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
    unsigned int textureFromFile(const char* path, const std::string& directory);
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
    Assimp::Importer importer;

    // Optimization flags for better performance
    unsigned int flags = aiProcess_Triangulate |
                        aiProcess_FlipUVs |
                        aiProcess_GenNormals |
                        aiProcess_OptimizeMeshes |
                        aiProcess_OptimizeGraph |
                        aiProcess_JoinIdenticalVertices |
                        aiProcess_ImproveCacheLocality |
                        aiProcess_RemoveRedundantMaterials;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);

    // Post-loading optimizations
    optimizeMeshes();
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

    // Process materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    return std::make_unique<Mesh>(vertices, indices, textures);
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

unsigned int Model::textureFromFile(const char* path, const std::string& directory) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

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

        // Optimization: Use anisotropic filtering if available
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

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

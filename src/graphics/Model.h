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
    unsigned int textureFromFile(const char* path, const std::string& modelDirectory);
    void calculateBoundingBox() const;
};

#endif //MODEL_H
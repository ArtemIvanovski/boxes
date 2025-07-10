#ifndef MESH_H
#define MESH_H

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "Shader.h"
#include "Material.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Material material;

    // Render data
    unsigned int VAO, VBO, EBO;

    // Performance optimization
    bool optimized = false;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
         std::vector<Texture> textures, Material material = Material());

    ~Mesh();

    // Render the mesh
    void draw(const Shader& shader) const;

    // Instanced rendering for better performance when drawing many identical objects
    void drawInstanced(const Shader& shader, unsigned int amount) const;

    // Optimization methods
    void optimize();

    // Getters for performance metrics
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }

    size_t getVertexCount() const { return vertices.size(); }
    size_t getTriangleCount() const { return indices.size() / 3; }

private:
    void setupMesh();
    void removeDuplicateVertices();
    void optimizeVertexCache();
};

#endif //MESH_H
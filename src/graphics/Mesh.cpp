#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<Texture> textures, Material material)
    : vertices(vertices), indices(indices), textures(textures), material(material) {
    setupMesh();
}

Mesh::~Mesh() {
    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::draw(const Shader& shader) const {
    // Передаем материал в шейдер
    shader.setVec3("material_ambient", material.ambient);
    shader.setVec3("material_diffuse", material.diffuse);
    shader.setVec3("material_specular", material.specular);
    shader.setFloat("material_shininess", material.shininess);

    // Bind appropriate textures
    bool hasDiffuseTexture = false;
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            hasDiffuseTexture = true;
        }
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.setInt((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    shader.setBool("has_diffuse_texture", hasDiffuseTexture);

    // Draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset to defaults
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::drawInstanced(const Shader& shader, unsigned int amount) const {
    // Bind textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        shader.setInt((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, amount);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::optimize() {
    if (optimized) return;

    // Remove duplicate vertices
    removeDuplicateVertices();

    // Optimize vertex cache
    optimizeVertexCache();

    // Regenerate OpenGL buffers with optimized data
    setupMesh();

    optimized = true;
}

void Mesh::setupMesh() {
    // Create buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Load vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Load index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}

void Mesh::removeDuplicateVertices() {
    std::unordered_map<std::string, unsigned int> uniqueVertices;
    std::vector<Vertex> newVertices;
    std::vector<unsigned int> newIndices;

    for (unsigned int index : indices) {
        const Vertex& vertex = vertices[index];

        // Create a hash key for the vertex
        std::string key = std::to_string(vertex.position.x) + "_" +
                         std::to_string(vertex.position.y) + "_" +
                         std::to_string(vertex.position.z) + "_" +
                         std::to_string(vertex.normal.x) + "_" +
                         std::to_string(vertex.normal.y) + "_" +
                         std::to_string(vertex.normal.z) + "_" +
                         std::to_string(vertex.texCoords.x) + "_" +
                         std::to_string(vertex.texCoords.y);

        if (uniqueVertices.count(key) == 0) {
            uniqueVertices[key] = static_cast<unsigned int>(newVertices.size());
            newVertices.push_back(vertex);
        }

        newIndices.push_back(uniqueVertices[key]);
    }

    vertices = std::move(newVertices);
    indices = std::move(newIndices);
}

void Mesh::optimizeVertexCache() {
    // Simple vertex cache optimization using a greedy approach
    // This improves GPU cache hit rates
    std::vector<unsigned int> optimizedIndices;
    std::vector<bool> used(vertices.size(), false);
    optimizedIndices.reserve(indices.size());

    for (size_t i = 0; i < indices.size(); i += 3) {
        // Process triangles to maximize cache efficiency
        unsigned int bestTriangle = i;
        int bestScore = -1;

        // Look ahead a few triangles to find the best next triangle
        for (size_t j = i; j < std::min(i + 12, indices.size()); j += 3) {
            int score = 0;
            for (int k = 0; k < 3; k++) {
                if (used[indices[j + k]]) score++;
            }
            if (score > bestScore) {
                bestScore = score;
                bestTriangle = j;
            }
        }

        // Add the best triangle
        for (int k = 0; k < 3; k++) {
            optimizedIndices.push_back(indices[bestTriangle + k]);
            used[indices[bestTriangle + k]] = true;
        }

        // Swap the triangle to current position
        if (bestTriangle != i) {
            for (int k = 0; k < 3; k++) {
                std::swap(indices[i + k], indices[bestTriangle + k]);
            }
        }
    }

    indices = std::move(optimizedIndices);
}
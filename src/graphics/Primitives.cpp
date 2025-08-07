#include "Primitives.h"
#include <vector>

std::unique_ptr<Mesh> Primitives::createBox(float width, float height, float depth, const Material& material) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;

    // Vertices для куба (24 вершины - по 4 для каждой грани)
    std::vector<glm::vec3> positions = {
        // Front face
        {-w, -h,  d}, { w, -h,  d}, { w,  h,  d}, {-w,  h,  d},
        // Back face
        {-w, -h, -d}, {-w,  h, -d}, { w,  h, -d}, { w, -h, -d},
        // Top face
        {-w,  h, -d}, {-w,  h,  d}, { w,  h,  d}, { w,  h, -d},
        // Bottom face
        {-w, -h, -d}, { w, -h, -d}, { w, -h,  d}, {-w, -h,  d},
        // Right face
        { w, -h, -d}, { w,  h, -d}, { w,  h,  d}, { w, -h,  d},
        // Left face
        {-w, -h, -d}, {-w, -h,  d}, {-w,  h,  d}, {-w,  h, -d}
    };

    std::vector<glm::vec3> normals = {
        // Front face
        {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
        // Back face
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
        // Top face
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
        // Bottom face
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0},
        // Right face
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
        // Left face
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}
    };

    for (int i = 0; i < 24; i++) {
        Vertex vertex;
        vertex.position = positions[i];
        vertex.normal = normals[i];
        vertex.texCoords = glm::vec2((i % 4) < 2 ? 0.0f : 1.0f, (i % 4) % 3 == 0 ? 0.0f : 1.0f);
        vertices.push_back(vertex);
    }

    // Indices для 12 треугольников (6 граней * 2 треугольника)
    std::vector<unsigned int> cubeIndices = {
        0,  1,  2,   0,  2,  3,   // front
        4,  5,  6,   4,  6,  7,   // back
        8,  9, 10,   8, 10, 11,   // top
        12, 13, 14,  12, 14, 15,  // bottom
        16, 17, 18,  16, 18, 19,  // right
        20, 21, 22,  20, 22, 23   // left
    };

    return std::make_unique<Mesh>(vertices, cubeIndices, std::vector<Texture>(), material);
}

std::unique_ptr<Mesh> Primitives::createGround(float width, float height, const Material& material) {
    // Простая плоскость
    std::vector<Vertex> vertices(4);
    vertices[0] = {{-width/2, 0, -height/2}, {0, 1, 0}, {0, 0}};
    vertices[1] = {{ width/2, 0, -height/2}, {0, 1, 0}, {1, 0}};
    vertices[2] = {{ width/2, 0,  height/2}, {0, 1, 0}, {1, 1}};
    vertices[3] = {{-width/2, 0,  height/2}, {0, 1, 0}, {0, 1}};

    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

    return std::make_unique<Mesh>(vertices, indices, std::vector<Texture>(), material);
}
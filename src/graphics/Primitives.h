#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "Mesh.h"
#include <memory>

class Primitives {
public:
    static std::unique_ptr<Mesh> createBox(float width, float height, float depth,
                                         const Material& material = Material());
    static std::unique_ptr<Mesh> createGround(float width, float height,
                                            const Material& material = Material());
};

#endif
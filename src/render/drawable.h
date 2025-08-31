#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "material.h"
#include "mesh.h"

struct Drawable {
    Mesh* mesh;
    Material* material;
    glm::mat4 model;
};

#endif //DRAWABLE_H

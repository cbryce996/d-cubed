#ifndef ENTITY_H
#define ENTITY_H

#include "camera.h"
#include "mesh.h"

struct Entity {
  std::string name;
  Transform transform;
  Mesh* mesh;
};

#endif //ENTITY_H

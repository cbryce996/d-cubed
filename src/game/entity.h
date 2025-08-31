#ifndef ENTITY_H
#define ENTITY_H

#include "camera.h"
#include "material.h"
#include "mesh.h"

struct Transform {
  glm::vec3 position = {0.0f, 0.0f, 5.0f};
  glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

struct Entity {
  std::string name;
  glm::mat4 model;
  Transform transform;
  Material* material;
  Mesh* mesh;
};

#endif //ENTITY_H

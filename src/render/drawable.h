#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <glm/glm.hpp>

struct Material;
struct Mesh;

struct Drawable {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	glm::mat4 model;
};

#endif // DRAWABLE_H

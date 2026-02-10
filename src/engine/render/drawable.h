#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "entity/entity.h"
#include "memory.h"
#include "mesh/mesh.h"

#include <vector>

struct Buffer;
struct MeshInstance;
struct MaterialInstance;

struct Drawable {
	MeshInstance* mesh = nullptr;
	MaterialInstance* material = nullptr;
	std::vector<Block> instance_blocks;
	glm::mat4 model = {1.0f};

	Buffer* instance_buffer = nullptr;
	Buffer* index_buffer = nullptr;
	Buffer* vertex_buffer = nullptr;
};

#endif // DRAWABLE_H

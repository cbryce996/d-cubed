#ifndef DRAWABLE_H
#define DRAWABLE_H
#include "material.h"

struct InstanceBatch {
	std::vector<Block> blocks;
};

struct Drawable {
	Mesh* mesh = nullptr;
	MaterialInstance* material = nullptr;
	InstanceBatch* instance_batch = nullptr;

	Buffer* instance_buffer = nullptr;
	Buffer* index_buffer = nullptr;
	Buffer* vertex_buffer = nullptr;
};

#endif // DRAWABLE_H

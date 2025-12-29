#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "buffer.h"
#include "instance.h"

struct Mesh;
struct Material;

struct Drawable {
	Mesh* mesh = nullptr;
	Material* material = nullptr;

	std::vector<Instance> instances;
	size_t instances_count;
	size_t instances_size;

	Buffer* instance_buffer = nullptr;
	Buffer* vertex_buffer = nullptr;
};

#endif // DRAWABLE_H

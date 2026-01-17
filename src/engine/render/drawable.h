#ifndef DRAWABLE_H
#define DRAWABLE_H

struct Mesh;
struct Material;

struct InstanceBatch {
	std::vector<Block> blocks;
};

struct Drawable {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	InstanceBatch* instance_batch = nullptr;

	Buffer* instance_buffer = nullptr;
	Buffer* index_buffer = nullptr;
	Buffer* vertex_buffer = nullptr;
};

#endif // DRAWABLE_H

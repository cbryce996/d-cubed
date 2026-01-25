#ifndef MESH_H
#define MESH_H

#define BLOCK_FLOATS 16
#define BLOCK_BYTES (BLOCK_FLOATS * sizeof (float))
#define ALIGNMENT 16

#include "maths/vector.h"

#include <glm/glm.hpp>

struct Vector3;

struct alignas (ALIGNMENT) Block {
	float data[BLOCK_FLOATS];
};

struct MeshCPUState {
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<uint32_t> indices;
};

struct MeshGPUState {
	std::vector<Block> vertices;
	std::vector<Block> normals;
	std::vector<uint32_t> indices;
};

struct MeshInstance {
	std::string name;
	MeshCPUState cpu_state;
	MeshGPUState gpu_state;
};

namespace MeshTransfer {
extern void to_gpu (MeshInstance& mesh);
extern void to_cpu (MeshInstance& mesh);
}

#endif // MESH_H

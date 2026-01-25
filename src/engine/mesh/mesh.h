#ifndef MESH_H
#define MESH_H

#include "maths/vector.h"
#include "render/memory.h"

#include <glm/glm.hpp>

struct Vector3;

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

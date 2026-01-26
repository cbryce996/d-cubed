#include "mesh.h"
#include "assets/asset.h"
#include "maths/geometry/sphere.h"

#include <cassert>

namespace MeshTransfer {

void to_gpu (MeshInstance& mesh) {
	assert (!mesh.cpu_state.vertices.empty ());
	assert (!mesh.cpu_state.normals.empty ());
	assert (!mesh.cpu_state.indices.empty ());

	assert (mesh.cpu_state.vertices.size () == mesh.cpu_state.normals.size ());

	assert (mesh.cpu_state.indices.size () <= UINT32_MAX);

	assert (sizeof (Block) % ALIGNMENT == 0);
	assert (alignof (Block) >= ALIGNMENT);

	mesh.gpu_state.vertices.resize (mesh.cpu_state.vertices.size ());

	assert (mesh.gpu_state.vertices.size () == mesh.cpu_state.vertices.size ());

	for (size_t i = 0; i < mesh.cpu_state.vertices.size (); ++i) {
		Block& b = mesh.gpu_state.vertices[i];

		assert (reinterpret_cast<uintptr_t> (&b) % ALIGNMENT == 0);

		b.data[0] = mesh.cpu_state.vertices[i].x;
		b.data[1] = mesh.cpu_state.vertices[i].y;
		b.data[2] = mesh.cpu_state.vertices[i].z;
		b.data[3] = 1.0f;

		b.data[4] = mesh.cpu_state.normals[i].x;
		b.data[5] = mesh.cpu_state.normals[i].y;
		b.data[6] = mesh.cpu_state.normals[i].z;
		b.data[7] = 0.0f;
	}

	mesh.gpu_state.indices = mesh.cpu_state.indices;

	assert (!mesh.gpu_state.vertices.empty ());
	assert (!mesh.gpu_state.indices.empty ());
}

void to_cpu (MeshInstance& mesh) {
	assert (!mesh.gpu_state.vertices.empty ());
	assert (!mesh.gpu_state.indices.empty ());

	assert (sizeof (Block) % ALIGNMENT == 0);
	assert (alignof (Block) >= ALIGNMENT);

	mesh.cpu_state.vertices.resize (mesh.gpu_state.vertices.size ());
	mesh.cpu_state.normals.resize (mesh.gpu_state.vertices.size ());

	assert (mesh.cpu_state.vertices.size () == mesh.gpu_state.vertices.size ());
	assert (mesh.cpu_state.normals.size () == mesh.gpu_state.vertices.size ());

	for (size_t i = 0; i < mesh.gpu_state.vertices.size (); ++i) {
		const Block& b = mesh.gpu_state.vertices[i];

		assert (reinterpret_cast<uintptr_t> (&b) % ALIGNMENT == 0);

		mesh.cpu_state.vertices[i].x = b.data[0];
		mesh.cpu_state.vertices[i].y = b.data[1];
		mesh.cpu_state.vertices[i].z = b.data[2];

		mesh.cpu_state.normals[i].x = b.data[4];
		mesh.cpu_state.normals[i].y = b.data[5];
		mesh.cpu_state.normals[i].z = b.data[6];
	}

	mesh.cpu_state.indices = mesh.gpu_state.indices;

	assert (!mesh.cpu_state.vertices.empty ());
	assert (!mesh.cpu_state.normals.empty ());
	assert (!mesh.cpu_state.indices.empty ());
}
}

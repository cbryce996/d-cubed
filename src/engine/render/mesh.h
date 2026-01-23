#ifndef MESH_H
#define MESH_H

#include "maths/vector.h"
#include "render/block.h"

#include <glm/glm.hpp>

struct Vector3;

struct Mesh {
	std::string name;
	std::vector<Vector3> vertices;
	std::vector<uint32_t> indices;
	std::vector<Vector3> normals;

	std::vector<Block> gpu_vertices;
	std::vector<uint32_t> gpu_indices;

	void to_gpu () {
		assert (vertices.size () == normals.size ());
		assert (!vertices.empty ());
		assert (!indices.empty ());

		gpu_vertices.resize (vertices.size ());

		for (size_t i = 0; i < vertices.size (); ++i) {
			Block& b = gpu_vertices[i];
			b.clear ();

			// position
			b.data[0] = vertices[i].x;
			b.data[1] = vertices[i].y;
			b.data[2] = vertices[i].z;
			b.data[3] = 1.0f;

			// normal
			b.data[4] = normals[i].x;
			b.data[5] = normals[i].y;
			b.data[6] = normals[i].z;
			b.data[7] = 0.0f;
		}

		gpu_indices = indices;
	}
};

#endif // MESH_H

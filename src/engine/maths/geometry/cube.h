#ifndef CUBE_H
#define CUBE_H

#include "../../mesh/mesh.h"
#include "maths/vector.h"

#include <cassert>
#include <vector>

namespace Cube {

struct FaceBasis {
	Vector3 normal;
	Vector3 tangent_u;
	Vector3 tangent_v;
};

inline void sample (float half_extent, std::vector<Vector3>& vertices) {
	assert (half_extent > 0.0f);

	vertices.clear ();
	vertices.reserve (24);

	const float h = half_extent;

	const Vector3 normals[6] = {{1, 0, 0},	{-1, 0, 0}, {0, 1, 0},
								{0, -1, 0}, {0, 0, 1},	{0, 0, -1}};

	const Vector3 tangents_u[6] = {{0, 0, -1}, {0, 0, 1}, {1, 0, 0},
								   {1, 0, 0},  {1, 0, 0}, {-1, 0, 0}};

	const float uv[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

	for (int f = 0; f < 6; ++f) {
		const Vector3 n = normals[f];
		const Vector3 tu = tangents_u[f];
		const Vector3 tv = cross (n, tu);

		for (const auto i : uv) {
			const float u = i[0];
			const float v = i[1];

			Vector3 p = n * h + tu * (u * h) + tv * (v * h);

			vertices.push_back (p);
		}
	}

	assert (vertices.size () == 24);
}

inline void build_indices (std::vector<uint32_t>& indices) {
	indices.clear ();
	indices.reserve (36);

	for (uint32_t face = 0; face < 6; ++face) {
		const uint32_t base = face * 4;

		indices.push_back (base + 0);
		indices.push_back (base + 1);
		indices.push_back (base + 2);

		indices.push_back (base + 0);
		indices.push_back (base + 2);
		indices.push_back (base + 3);
	}

	assert (indices.size () == 36);
}

inline void calculate_normals (
	const std::vector<Vector3>& vertices, std::vector<Vector3>& normals
) {
	assert (!vertices.empty ());

	normals.clear ();
	normals.reserve (vertices.size ());

	for (size_t i = 0; i < vertices.size (); i += 4) {
		Vector3 n = normalise (
			cross (vertices[i + 1] - vertices[i], vertices[i + 2] - vertices[i])
		);

		for (int j = 0; j < 4; ++j) {
			normals.push_back (n);
		}
	}

	assert (normals.size () == vertices.size ());
}

inline MeshInstance generate (const float size) {
	assert (size > 0.0f);

	MeshInstance cube{};
	cube.name = "Cube";

	const float h = size * 0.5f;

	sample (h, cube.cpu_state.vertices);
	build_indices (cube.cpu_state.indices);
	calculate_normals (cube.cpu_state.vertices, cube.cpu_state.normals);

	assert (!cube.cpu_state.vertices.empty ());
	assert (cube.cpu_state.vertices.size () == cube.cpu_state.normals.size ());
	assert (!cube.cpu_state.indices.empty ());
	assert (cube.cpu_state.indices.size () % 3 == 0);

	return cube;
}

}

#endif // CUBE_H

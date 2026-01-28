#ifndef PLANE_H
#define PLANE_H

#include "mesh/mesh.h"

#include <cassert>
#include <vector>

namespace Plane {
inline MeshInstance generate (float size, uint32_t resolution = 1) {
	assert (size > 0.0f);
	assert (resolution >= 1);

	MeshInstance plane{};
	plane.name = "Plane";

	const float half = size * 0.5f;
	const uint32_t verts_per_side = resolution + 1;
	const float step = size / resolution;

	auto& verts = plane.cpu_state.vertices;
	auto& norms = plane.cpu_state.normals;
	auto& indices = plane.cpu_state.indices;

	verts.clear ();
	norms.clear ();
	indices.clear ();

	verts.reserve (verts_per_side * verts_per_side);
	norms.reserve (verts_per_side * verts_per_side);
	indices.reserve (resolution * resolution * 6);

	// === Vertices + normals ===
	for (uint32_t z = 0; z <= resolution; ++z) {
		for (uint32_t x = 0; x <= resolution; ++x) {
			const float px = -half + x * step;
			const float pz = -half + z * step;

			verts.emplace_back (px, 0.0f, pz);
			norms.emplace_back (0.0f, -1.0f, 0.0f);
		}
	}

	// === Indices ===
	for (uint32_t z = 0; z < resolution; ++z) {
		for (uint32_t x = 0; x < resolution; ++x) {
			const uint32_t i0 = z * verts_per_side + x;
			const uint32_t i1 = i0 + 1;
			const uint32_t i2 = i0 + verts_per_side;
			const uint32_t i3 = i2 + 1;

			indices.push_back (i0);
			indices.push_back (i1);
			indices.push_back (i2);

			indices.push_back (i1);
			indices.push_back (i3);
			indices.push_back (i2);
		}
	}

	assert (!verts.empty ());
	assert (verts.size () == norms.size ());
	assert (!indices.empty ());
	assert (indices.size () % 3 == 0);

	return plane;
}
}

#endif // PLANE_H

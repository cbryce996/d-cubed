#ifndef PLANET_H
#define PLANET_H

#include "maths/vector.h"
#include "render/mesh.h"

#include <cassert>
#include <cmath>
#include <vector>

namespace Sphere {

inline void sample_unit_sphere(
    float radius,
    int lat_steps,
    int long_steps,
    std::vector<Vector3>& vertices
) {
    assert(radius > 0.0f);
    assert(lat_steps >= 2);
    assert(long_steps >= 3);

    vertices.clear();

    const int R = long_steps + 1;
	const int ring_count = lat_steps - 1;
    const int total = 2 + ring_count * R;

    vertices.reserve(total);

    vertices.push_back({0.0f, radius, 0.0f});

    for (int row = 1; row < lat_steps; ++row) {
        const float phi = static_cast<float>(M_PI) * static_cast<float> (row) / static_cast<float> (lat_steps);

        for (int col = 0; col <= long_steps; ++col) {
            const float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float> (col) / static_cast<float> (long_steps);

            Vector3 unit = {
                std::sin(phi) * std::cos(theta),
                std::cos(phi),
                std::sin(phi) * std::sin(theta)
            };

            vertices.push_back(unit * radius);
        }
    }

    vertices.push_back({0.0f, -radius, 0.0f});

    assert(static_cast<int> (vertices.size ()) == total);
}

inline void build_indices(
    int lat_steps,
    int long_steps,
    std::vector<uint32_t>& indices
) {
    assert(lat_steps >= 2);
    assert(long_steps >= 3);

    indices.clear();

    const int R = long_steps + 1;
    const int ring_count = lat_steps - 1;

	constexpr uint32_t first_ring = 1;
    const uint32_t south = first_ring + ring_count * R;

    const int top_tris = long_steps;
    const int mid_tris = (ring_count - 1) * long_steps * 2;
    const int bot_tris = long_steps;

    indices.reserve((top_tris + mid_tris + bot_tris) * 3);

    for (int col = 0; col < long_steps; ++col) {
		constexpr uint32_t north = 0;
		indices.push_back(north);
        indices.push_back(first_ring + col + 1);
        indices.push_back(first_ring + col);
    }

    for (int ring = 0; ring < ring_count - 1; ++ring) {
        const uint32_t ring0 = first_ring + ring * R;
        const uint32_t ring1 = ring0 + R;

        for (int col = 0; col < long_steps; ++col) {
            const uint32_t ul = ring0 + col;
            const uint32_t ur = ring0 + col + 1;
            const uint32_t ll = ring1 + col;
            const uint32_t lr = ring1 + col + 1;

            indices.push_back(ul);
            indices.push_back(ur);
            indices.push_back(ll);

            indices.push_back(ur);
            indices.push_back(lr);
            indices.push_back(ll);
        }
    }

    const uint32_t last_ring = first_ring + (ring_count - 1) * R;

    for (int col = 0; col < long_steps; ++col) {
        indices.push_back(last_ring + col);
        indices.push_back(last_ring + col + 1);
        indices.push_back(south);
    }
}

inline void calculate_normals(
    const std::vector<Vector3>& vertices,
    std::vector<Vector3>& normals
) {
	assert (!vertices.empty ());

    normals.clear();
    normals.reserve(vertices.size());

    for (const Vector3& v : vertices) {
        normals.push_back(normalise(v));
    }
}

inline Mesh generate (const float radius, const int lat_steps, const int long_steps) {
	assert(radius > 0.0f);
	assert (lat_steps > 0);
	assert (long_steps > 0);

    Mesh sphere{};
    sphere.name = "Sphere";

    sample_unit_sphere(radius, lat_steps, long_steps, sphere.vertices);
    build_indices(lat_steps, long_steps, sphere.indices);
    calculate_normals(sphere.vertices, sphere.normals);

    assert(!sphere.vertices.empty());
    assert(sphere.vertices.size() == sphere.normals.size());
    assert(!sphere.indices.empty());
    assert(sphere.indices.size() % 3 == 0);

    return sphere;
}
}

#endif // PLANET_H

#ifndef MESH_H
#define MESH_H

#include "memory.h"

#include <glm/glm.hpp>

#include "pipeline.h"
#include "render.h"

constexpr Collection triangle_vertices[] = {
	{{0.0f, 0.5f, 0.0f}}, {{0.5f, -0.5f, 0.0f}}, {{-0.5f, -0.5f, 0.0f}}
};

inline Collection make_vertex (
	const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& color
) {
	Collection vertex{};
	vertex.push (glm::vec4 (pos, 0.0f));	// Block 0: position
	vertex.push (glm::vec4 (normal, 0.0f)); // Block 1: normal
	vertex.push (glm::vec4 (color, 0.0f));	// Block 2: color
	vertex.push (glm::vec4 (0.0f));			// Block 3: padding
	return vertex;
}

// Full cube vertices (36 vertices, 6 faces x 2 triangles x 3 vertices)
static Collection cube_vertices[] = {
	// Front face (Z+) - Red
	make_vertex ({-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),
	make_vertex ({0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),
	make_vertex ({0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),

	make_vertex ({0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),
	make_vertex ({-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),
	make_vertex ({-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}),

	// Back face (Z-) - Green
	make_vertex ({-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),
	make_vertex ({-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),
	make_vertex ({0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),

	make_vertex ({0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),
	make_vertex ({0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),
	make_vertex ({-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}),

	// Left face (X-) - Blue
	make_vertex ({-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}),
	make_vertex ({-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}),
	make_vertex ({-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}),

	make_vertex ({-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}),
	make_vertex ({-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}),
	make_vertex ({-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}),

	// Right face (X+) - Yellow
	make_vertex ({0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}),
	make_vertex ({0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}),
	make_vertex ({0.5f, -0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}),

	make_vertex ({0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}),
	make_vertex ({0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}),
	make_vertex ({0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}),

	// Top face (Y+) - Magenta
	make_vertex ({-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}),
	make_vertex ({-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}),
	make_vertex ({0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}),

	make_vertex ({0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}),
	make_vertex ({0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}),
	make_vertex ({-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}),

	// Bottom face (Y-) - Cyan
	make_vertex ({-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}),
	make_vertex ({0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}),
	make_vertex ({-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}),

	make_vertex ({0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}),
	make_vertex ({-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}),
	make_vertex ({0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}),
};

constexpr size_t cube_vertex_count = sizeof (cube_vertices)
									 / sizeof (Collection);

struct Mesh {
	std::string name;
	const Block* vertex_data;
	size_t vertex_size;
	size_t vertex_count;
	std::shared_ptr<std::vector<Block>> vertex_storage;
};

inline std::shared_ptr<Mesh> create_cube_mesh () {
	auto mesh = std::make_shared<Mesh> ();
	mesh->name = "cube";

	// Create a storage vector for pure 64-byte Data objects
	mesh->vertex_storage = std::make_shared<std::vector<Block>> ();
	mesh->vertex_storage->reserve (cube_vertex_count);

	for (size_t i = 0; i < cube_vertex_count; ++i) {
		// Extract the .storage member (the 64-byte part) from the Collection
		// builder
		mesh->vertex_storage->push_back (cube_vertices[i].storage);
	}

	mesh->vertex_data = mesh->vertex_storage->data ();
	mesh->vertex_count = mesh->vertex_storage->size ();

	// vertex_size is now exactly vertex_count * 64
	mesh->vertex_size = mesh->vertex_storage->size () * sizeof (Block);

	return mesh;
}

#endif // MESH_H

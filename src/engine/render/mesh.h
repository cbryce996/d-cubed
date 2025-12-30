#ifndef MESH_H
#define MESH_H

#include "memory.h"

#include <glm/glm.hpp>

#include "pipeline.h"
#include "render.h"

constexpr Vertex triangle_vertices[] = {
	{{0.0f, 0.5f, 0.0f}}, {{0.5f, -0.5f, 0.0f}}, {{-0.5f, -0.5f, 0.0f}}
};

inline Vertex make_vertex (
	const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& color
) {
	Vertex vertex{};
	Vertex::push (&vertex, glm::vec4 (pos, 0.0f));	  // Block 0: position
	Vertex::push (&vertex, glm::vec4 (normal, 0.0f)); // Block 1: normal
	Vertex::push (&vertex, glm::vec4 (color, 0.0f));  // Block 2: color
	Vertex::push (&vertex, glm::vec4 (0.0f));		  // Block 3: padding
	return vertex;
}

// Full cube vertices (36 vertices, 6 faces x 2 triangles x 3 vertices)
static Vertex cube_vertices[] = {
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

constexpr size_t cube_vertex_count = sizeof (cube_vertices) / sizeof (Vertex);

struct Mesh {
	std::string name;
	const Vertex* vertex_data;
	size_t vertex_size;
	size_t vertex_count;
	std::shared_ptr<std::vector<Vertex>> vertex_storage;
};

inline std::shared_ptr<Mesh> create_cube_mesh () {
	auto mesh = std::make_shared<Mesh> ();
	mesh->name = "cube";

	mesh->vertex_storage = std::make_shared<std::vector<Vertex>> (
		std::begin (cube_vertices), std::end (cube_vertices)
	);

	mesh->vertex_data = mesh->vertex_storage->data ();
	mesh->vertex_count = mesh->vertex_storage->size ();
	mesh->vertex_size = mesh->vertex_storage->size () * sizeof (Vertex);

	return mesh;
}

#endif // MESH_H

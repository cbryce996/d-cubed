#ifndef MESH_H
#define MESH_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "pipeline.h"

struct Uniform {
	glm::mat4 model;
	glm::mat4 mvp;
	glm::vec3 light_pos;
	float pad;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

constexpr Vertex triangle_vertices[] = {{{0.0f, 0.5f, 0.0f}}, {{0.5f, -0.5f, 0.0f}}, {{-0.5f, -0.5f, 0.0f}}};

constexpr Vertex cube_vertices[] = {
	// Front face (Z+) - Red
	{{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},
	{{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},
	{{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},
	{{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},
	{{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},
	{{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0, 0}},

	// Back face (Z-) - Green
	{{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
	{{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
	{{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
	{{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
	{{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
	{{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},

	// Left face (X-) - Blue
	{{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}},
	{{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}},
	{{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}},
	{{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {0, 0, 1}},
	{{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}},
	{{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0, 1}},

	// Right face (X+) - Yellow
	{{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}},
	{{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}},
	{{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}},
	{{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}},
	{{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {1, 1, 0}},
	{{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 1, 0}},

	// Top face (Y+) - Magenta
	{{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}},
	{{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}},
	{{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}},
	{{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0, 1}},
	{{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}},
	{{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 0, 1}},

	// Bottom face (Y-) - Cyan
	{{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}},
	{{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}},
	{{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}},
	{{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}},
	{{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 1, 1}},
	{{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1, 1}},
};

constexpr size_t cube_vertex_count = sizeof(cube_vertices) / sizeof(Vertex);

struct Mesh {
	std::string name;
	const Vertex* vertex_data;
	size_t vertex_size;
	size_t vertex_count;
};

inline Mesh create_cube_mesh() {
	return Mesh{
		.name = "cube",
		.vertex_data = cube_vertices,
		.vertex_size = sizeof(cube_vertices),
		.vertex_count = cube_vertex_count
	};
}

#endif	// MESH_H

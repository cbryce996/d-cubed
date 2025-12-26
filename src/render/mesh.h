#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	std::shared_ptr<std::vector<Vertex>> vertex_storage;
};

inline std::shared_ptr<Mesh> create_cube_mesh() {
	auto mesh = std::make_shared<Mesh>();
	mesh->name = "cube";

	mesh->vertex_storage = std::make_shared<std::vector<Vertex>>(
		std::begin(cube_vertices), std::end(cube_vertices)
	);

	mesh->vertex_data  = mesh->vertex_storage->data();
	mesh->vertex_count = mesh->vertex_storage->size();
	mesh->vertex_size  = mesh->vertex_storage->size() * sizeof(Vertex);

	return mesh;
}

#endif	// MESH_H

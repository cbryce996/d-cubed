#define TINYOBJLOADER_IMPLEMENTATION
#include "loader.h"

#include "tiny_obj_loader.h"

bool TinyObjMeshLoader::load(
	const std::string& path,
	std::vector<Vertex>& out_vertices
) {
	out_vertices.clear();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&warn,
			&err,
			path.c_str()
		)) {
		SDL_LogError(
			SDL_LOG_CATEGORY_APPLICATION,
			"Failed to load %s",
			path.c_str(),
			err.c_str()
		);
		return false;
	}

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex v{};
			v.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			if (!attrib.normals.empty()) {
				v.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}
			if (!attrib.colors.empty()) {
				v.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
				};
			} else {
				v.color = {1.0f, 1.0f, 1.0f};
			}

			// Optional: assign UVs or other attributes if your Vertex has them
			out_vertices.push_back(v);
		}
	}

	return true;
}
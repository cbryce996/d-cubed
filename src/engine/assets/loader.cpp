#define TINYOBJLOADER_IMPLEMENTATION

#include <SDL3/SDL.h>

#include "loader.h"
#include "tiny_obj_loader.h"

bool TinyObjMeshLoader::load (
	const std::string& path, std::vector<Vector3>& out_vertices
) {
	out_vertices.clear ();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj (
			&attrib, &shapes, &materials, &warn, &err, path.c_str ()
		)) {
		SDL_LogError (
			SDL_LOG_CATEGORY_APPLICATION, "Failed to load %s", path.c_str (),
			err.c_str ()
		);
		return false;
	}

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			// TODO: Implment
		}
	}

	return true;
}

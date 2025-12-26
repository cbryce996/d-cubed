#include <thread>
#include <SDL3/SDL_log.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "asset.h"


std::shared_ptr<Mesh> AssetManager::load_mesh_from_file(const std::string& path) {
    auto mesh = std::make_shared<Mesh>();
    mesh->name = path;

    std::vector<Vertex> vertices;

    if (path == "cube.obj") {
        vertices.assign(std::begin(cube_vertices), std::end(cube_vertices));
    } else {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load %s: %s", path.c_str(), err.c_str());
            return nullptr;
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
                    v.color = {1.0f, 1.0f, 1.0f}; // default white
                }
                vertices.push_back(v);
            }
        }
    }

	mesh->vertex_storage = std::make_shared<std::vector<Vertex>>(std::move(vertices));
	mesh->vertex_data = mesh->vertex_storage->data();
	mesh->vertex_count = mesh->vertex_storage->size();
	mesh->vertex_size = mesh->vertex_storage->size() * sizeof(Vertex);

	{
    	std::scoped_lock lock(mesh_mutex);
    	meshes[path] = mesh;
	}

    SDL_Log("Loaded mesh: %s (%zu vertices)", path.c_str(), mesh->vertex_count);
    return mesh;
}

std::shared_ptr<Mesh> AssetManager::load_mesh(const std::string& path) {
	std::lock_guard lock(mesh_mutex);

	auto it = meshes.find(path);
	if (it != meshes.end()) {
		return it->second; // already loaded
	}

	auto mesh = load_mesh_from_file(path);
	meshes[path] = mesh;
	return mesh;
}

void AssetManager::reload_mesh(const std::string& path) {
	std::thread([this, path]() {
		auto new_mesh = load_mesh_from_file(path);

		{
			std::lock_guard lock(mesh_mutex);
			meshes[path] = new_mesh; // atomic swap
		}

		SDL_Log("Hot-reloaded mesh: %s", path.c_str());
	}).detach();
}

std::shared_ptr<Mesh> AssetManager::get_mesh(const std::string& path) {
	std::lock_guard lock(mesh_mutex);
	auto it = meshes.find(path);
	return it != meshes.end() ? it->second : nullptr;
}
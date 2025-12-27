#include <SDL3/SDL_log.h>
#include <thread>

#include "engine/assets/asset.h"

AssetManager::AssetManager(std::shared_ptr<IMeshLoader> loader)
	: loader(std::move(loader)) {}

std::shared_ptr<Mesh> AssetManager::load_mesh_from_file(
	const std::string& path
) {
	auto mesh = std::make_shared<Mesh>();
	mesh->name = path;

	std::vector<Vertex> vertices;

	if (!loader->load(path, vertices)) {
		SDL_LogError(
			SDL_LOG_CATEGORY_APPLICATION,
			"Failed to load %s",
			path.c_str()
		);
		return nullptr;
	}

	mesh->vertex_storage =
		std::make_shared<std::vector<Vertex>>(std::move(vertices));
	mesh->vertex_data = mesh->vertex_storage->data();
	mesh->vertex_count = mesh->vertex_storage->size();
	mesh->vertex_size = mesh->vertex_storage->size() * sizeof(Vertex);
	meshes[path] = mesh;

	SDL_Log("Loaded mesh: %s (%zu vertices)", path.c_str(), mesh->vertex_count);
	return mesh;
}

std::shared_ptr<Mesh> AssetManager::load_mesh(const std::string& path) {
	std::lock_guard lock(mesh_mutex);

	auto it = meshes.find(path);
	if (it != meshes.end()) {
		return it->second;
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
			meshes[path] = new_mesh;
		}

		SDL_Log("Hot-reloaded mesh: %s", path.c_str());
	}).detach();
}

std::shared_ptr<Mesh> AssetManager::get_mesh(const std::string& path) {
	std::lock_guard lock(mesh_mutex);
	auto it = meshes.find(path);
	return it != meshes.end() ? it->second : nullptr;
}
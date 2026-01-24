#include "engine/assets/asset.h"

#include "../mesh/mesh.h"

#include <SDL3/SDL_log.h>

#include <thread>

AssetManager::AssetManager (std::shared_ptr<IMeshLoader> loader)
	: loader (std::move (loader)) {}

std::shared_ptr<MeshInstance>
AssetManager::load_mesh_from_file (const std::string& path) {
	auto mesh = std::make_shared<MeshInstance> ();
	mesh->name = path;

	std::vector<Vector3> vertices;

	if (!loader->load (path, vertices)) {
		SDL_LogError (
			SDL_LOG_CATEGORY_APPLICATION, "Failed to load %s", path.c_str ()
		);
		return nullptr;
	}

	mesh->cpu_state.vertices = vertices;
	meshes[path] = mesh;

	return mesh;
}

std::shared_ptr<MeshInstance>
AssetManager::load_mesh (const std::string& path) {
	std::lock_guard lock (mesh_mutex);

	auto iterator = meshes.find (path);
	if (iterator != meshes.end ())
		return iterator->second;

	auto mesh = load_mesh_from_file (path);
	meshes[path] = mesh;
	return mesh;
}

void AssetManager::reload_mesh (const std::string& path) {
	std::thread ([this, path] () {
		auto new_mesh = load_mesh_from_file (path);

		{
			std::lock_guard lock (mesh_mutex);
			meshes[path] = new_mesh;
		}

		SDL_Log ("Hot-reloaded mesh: %s", path.c_str ());
	}).detach ();
}

std::shared_ptr<MeshInstance> AssetManager::get_mesh (const std::string& path) {
	std::lock_guard lock (mesh_mutex);
	auto iterator = meshes.find (path);
	return iterator != meshes.end () ? iterator->second : nullptr;
}

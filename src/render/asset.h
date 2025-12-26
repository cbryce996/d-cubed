#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "mesh.h"

class AssetManager {
public:
	AssetManager() = default;

	std::shared_ptr<Mesh> load_mesh(const std::string& path);
	void reload_mesh(const std::string& path);
	std::shared_ptr<Mesh> get_mesh(const std::string& path);

private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
	std::mutex mesh_mutex;

	std::shared_ptr<Mesh> load_mesh_from_file(const std::string& path);
};

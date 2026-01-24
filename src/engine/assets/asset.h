#pragma once

#include "loader.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

struct MeshInstance;
struct Mesh;

class AssetManager {
  public:
	explicit AssetManager (
		std::shared_ptr<IMeshLoader> loader
		= std::make_shared<TinyObjMeshLoader> ()
	);
	std::shared_ptr<MeshInstance> load_mesh (const std::string& path);
	void reload_mesh (const std::string& path);
	std::shared_ptr<MeshInstance> get_mesh (const std::string& path);

  private:
	std::unordered_map<std::string, std::shared_ptr<MeshInstance>> meshes;
	std::mutex mesh_mutex;

	std::shared_ptr<MeshInstance> load_mesh_from_file (const std::string& path);
	std::shared_ptr<IMeshLoader> loader;
};

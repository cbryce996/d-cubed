#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "assets/mesh/mesh.h"
#include "core/math/vector.h"
#include "engine/assets/asset.h"

struct FakeLoader : IMeshLoader {
	bool load (
		const std::string& path, std::vector<Vector3>& out_vertices
	) override {
		out_vertices = {{{0, 0, 0}, {0, 1, 0}, {1, 1, 1}}}; // dummy vertex
		return true;
	}
};

class AssetManagerTest : public ::testing::Test {
  protected:
	void SetUp () override {}
};

TEST_F (AssetManagerTest, LoadsMeshSuccessfully) {
	const std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	auto* asset_manager = new AssetManager (loader);

	const std::shared_ptr<MeshInstance> mesh = asset_manager->load_mesh (
		"cube.obj"
	);
	ASSERT_NE (mesh, nullptr);
	EXPECT_GT (mesh->cpu_state.vertices.size (), 0);
	EXPECT_EQ (mesh->name, "cube.obj");
}

TEST_F (AssetManagerTest, CachesLoadedMesh) {
	const std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	auto* asset_manager = new AssetManager (loader);

	const std::shared_ptr<MeshInstance> mesh1 = asset_manager->load_mesh (
		"cube.obj"
	);
	const std::shared_ptr<MeshInstance> mesh2 = asset_manager->load_mesh (
		"cube.obj"
	);
	ASSERT_EQ (mesh1, mesh2);
}

TEST_F (AssetManagerTest, RetrievesLoadedMesh) {
	const std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	auto* asset_manager = new AssetManager (loader);

	asset_manager->load_mesh ("cube.obj");
	const std::shared_ptr<MeshInstance> mesh = asset_manager->get_mesh (
		"cube.obj"
	);
	ASSERT_NE (mesh, nullptr);
	EXPECT_EQ (mesh->name, "cube.obj");
}

TEST_F (AssetManagerTest, ReturnsNullForMissingMesh) {
	const std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	auto* asset_manager = new AssetManager (loader);

	const std::shared_ptr<MeshInstance> mesh = asset_manager->get_mesh (
		"null.obj"
	);
	EXPECT_EQ (mesh, nullptr);
}

TEST_F (AssetManagerTest, ReloadsMeshUpdatesCache) {
	const std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	auto* asset_manager = new AssetManager (loader);

	const std::shared_ptr<MeshInstance> mesh1 = asset_manager->load_mesh (
		"cube.obj"
	);
	asset_manager->reload_mesh ("cube.obj");

	std::this_thread::sleep_for (std::chrono::milliseconds (100));

	const std::shared_ptr<MeshInstance> mesh2 = asset_manager->get_mesh (
		"cube.obj"
	);
	ASSERT_NE (mesh2, nullptr);
	EXPECT_NE (mesh1, mesh2);
}

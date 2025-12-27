#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "../src/engine/assets/asset.h"

struct FakeLoader : IMeshLoader {
	bool
	load (const std::string& path, std::vector<Vertex>& out_vertices) override {
		out_vertices = {{{0, 0, 0}, {0, 1, 0}, {1, 1, 1}}}; // dummy vertex
		return true;
	}
};

class AssetManagerTest : public ::testing::Test {
  protected:
	void SetUp () override {}
};

TEST_F (AssetManagerTest, LoadsMeshSuccessfully) {
	std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	AssetManager* asset_manager = new AssetManager (loader);

	std::shared_ptr<Mesh> mesh = asset_manager->load_mesh ("cube.obj");
	ASSERT_NE (mesh, nullptr) << "Expected mesh to not be null";
	EXPECT_GT (mesh->vertex_count, 0) << "Expected loaded mesh to have "
										 "vertices";
	EXPECT_EQ (mesh->name, "cube.obj") << "Expected mesh to have name";
}

TEST_F (AssetManagerTest, CachesLoadedMesh) {
	std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	AssetManager* asset_manager = new AssetManager (loader);

	std::shared_ptr<Mesh> mesh1 = asset_manager->load_mesh ("cube.obj");
	std::shared_ptr<Mesh> mesh2 = asset_manager->load_mesh ("cube.obj");
	ASSERT_EQ (mesh1, mesh2) << "Expected cached mesh";
}

TEST_F (AssetManagerTest, RetrievesLoadedMesh) {
	std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	AssetManager* asset_manager = new AssetManager (loader);

	asset_manager->load_mesh ("cube.obj");
	std::shared_ptr<Mesh> mesh = asset_manager->get_mesh ("cube.obj");
	ASSERT_NE (mesh, nullptr) << "Expected mesh to not be null";
	EXPECT_EQ (mesh->name, "cube.obj") << "Expected mesh to have name'";
}

TEST_F (AssetManagerTest, ReturnsNullForMissingMesh) {
	std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	AssetManager* asset_manager = new AssetManager (loader);

	std::shared_ptr<Mesh> mesh = asset_manager->get_mesh ("null.obj");
	EXPECT_EQ (mesh, nullptr) << "Expected mesh to be null";
}

TEST_F (AssetManagerTest, ReloadsMeshUpdatesCache) {
	std::shared_ptr<IMeshLoader> loader = std::make_shared<FakeLoader> ();
	AssetManager* asset_manager = new AssetManager (loader);

	std::shared_ptr<Mesh> mesh1 = asset_manager->load_mesh ("cube.obj");
	asset_manager->reload_mesh ("cube.obj");

	std::this_thread::sleep_for (std::chrono::milliseconds (100));

	std::shared_ptr<Mesh> mesh2 = asset_manager->get_mesh ("cube.obj");
	ASSERT_NE (mesh2, nullptr) << "Expected mesh to not be null";
	EXPECT_NE (mesh1, mesh2) << "Expected mesh to not be cached";
}

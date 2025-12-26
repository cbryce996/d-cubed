#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "../src/engine/assets/asset.h"

struct FakeLoader : IMeshLoader {
	bool load(const std::string& path, std::vector<Vertex>& out_vertices) override {
		out_vertices = { {{0,0,0}, {0,1,0}, {1,1,1}} }; // dummy vertex
		return true;
	}
};

class AssetManagerTest : public ::testing::Test {
protected:
	void SetUp() override {
		asset_manager = std::make_unique<AssetManager>(std::make_shared<FakeLoader>());
	}

	std::unique_ptr<AssetManager> asset_manager;
};

TEST_F(AssetManagerTest, LoadCubeMesh) {
	auto mesh = asset_manager->load_mesh("cube.obj");
	ASSERT_NE(mesh, nullptr); // should not be null
	EXPECT_GT(mesh->vertex_count, 0); // should have vertices
	EXPECT_EQ(mesh->name, "cube.obj");
}

TEST_F(AssetManagerTest, LoadMeshCaching) {
	auto mesh1 = asset_manager->load_mesh("cube.obj");
	auto mesh2 = asset_manager->load_mesh("cube.obj");
	ASSERT_EQ(mesh1, mesh2); // should return the same shared_ptr
}

TEST_F(AssetManagerTest, GetMeshReturnsLoadedMesh) {
	asset_manager->load_mesh("cube.obj");
	auto mesh = asset_manager->get_mesh("cube.obj");
	ASSERT_NE(mesh, nullptr);
	EXPECT_EQ(mesh->name, "cube.obj");
}

TEST_F(AssetManagerTest, GetMeshReturnsNullForMissingMesh) {
	auto mesh = asset_manager->get_mesh("nonexistent.obj");
	EXPECT_EQ(mesh, nullptr);
}

TEST_F(AssetManagerTest, ReloadMeshUpdatesMesh) {
	auto mesh1 = asset_manager->load_mesh("cube.obj");
	asset_manager->reload_mesh("cube.obj");

	// Give hot-reload thread some time to complete
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	auto mesh2 = asset_manager->get_mesh("cube.obj");
	ASSERT_NE(mesh2, nullptr);

	// The mesh object should be different after reload
	EXPECT_NE(mesh1, mesh2);
}

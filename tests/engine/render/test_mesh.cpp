#include "render/mesh.h"
#include <gtest/gtest.h>

class MeshTest : public ::testing::Test {
  protected:
	Mesh mesh;

	void SetUp () override {
		mesh.name = "test_mesh";
		mesh.vertices.clear ();
		mesh.normals.clear ();
		mesh.indices.clear ();
		mesh.gpu_vertices.clear ();
		mesh.gpu_indices.clear ();
	}

	void MakeSimpleTriangle () {
		mesh.vertices = {{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}};

		mesh.normals = {{0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}};

		mesh.indices = {0, 1, 2};
	}
};

TEST_F (MeshTest, ToGpuCopiesIndices) {
	MakeSimpleTriangle ();

	mesh.to_gpu ();

	EXPECT_EQ (mesh.gpu_indices.size (), mesh.indices.size ());
	EXPECT_EQ (mesh.gpu_indices, mesh.indices);
}

TEST_F (MeshTest, ToGpuCreatesCorrectVertexCount) {
	MakeSimpleTriangle ();

	mesh.to_gpu ();

	EXPECT_EQ (mesh.gpu_vertices.size (), mesh.vertices.size ());
}

TEST_F (MeshTest, ToGpuPacksPositionCorrectly) {
	MakeSimpleTriangle ();

	mesh.to_gpu ();

	const Block& b = mesh.gpu_vertices[0];

	EXPECT_FLOAT_EQ (b.data[0], mesh.vertices[0].x);
	EXPECT_FLOAT_EQ (b.data[1], mesh.vertices[0].y);
	EXPECT_FLOAT_EQ (b.data[2], mesh.vertices[0].z);
	EXPECT_FLOAT_EQ (b.data[3], 1.0f);
}

TEST_F (MeshTest, ToGpuPacksNormalCorrectly) {
	MakeSimpleTriangle ();

	mesh.to_gpu ();

	const auto& [data] = mesh.gpu_vertices[0];

	EXPECT_FLOAT_EQ (data[4], mesh.normals[0].x);
	EXPECT_FLOAT_EQ (data[5], mesh.normals[0].y);
	EXPECT_FLOAT_EQ (data[6], mesh.normals[0].z);
	EXPECT_FLOAT_EQ (data[7], 0.0f);
}

TEST_F (MeshTest, ToGpuClearsAndRebuildsGpuBuffers) {
	MakeSimpleTriangle ();
	mesh.to_gpu ();

	auto first_gpu_vertices = mesh.gpu_vertices;
	auto first_gpu_indices = mesh.gpu_indices;

	mesh.vertices[0] = {2.f, 2.f, 2.f};
	mesh.normals[0] = {1.f, 0.f, 0.f};

	mesh.to_gpu ();

	const auto& [data] = mesh.gpu_vertices[0];

	EXPECT_NE (data[0], first_gpu_vertices[0].data[0]);
	EXPECT_NE (data[4], first_gpu_vertices[0].data[4]);
}

TEST_F (MeshTest, ToGpuRequiresVerticesAndNormalsMatch) {
	mesh.vertices = {{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}};

	mesh.normals = {{0.f, 0.f, 1.f}};

	mesh.indices = {0, 1};

	EXPECT_DEATH (mesh.to_gpu (), ".*");
}

TEST_F (MeshTest, ToGpuRequiresVertices) {
	mesh.normals = {{0.f, 0.f, 1.f}};
	mesh.indices = {0};

	EXPECT_DEATH (mesh.to_gpu (), ".*");
}

TEST_F (MeshTest, ToGpuRequiresIndices) {
	mesh.vertices = {{0.f, 0.f, 0.f}};
	mesh.normals = {{0.f, 0.f, 1.f}};

	EXPECT_DEATH (mesh.to_gpu (), ".*");
}

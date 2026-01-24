#include "mesh/mesh.h"
#include <cstdint>
#include <gtest/gtest.h>

class MeshTransferAlignedTest : public ::testing::Test {
  protected:
	MeshInstance mesh;

	void SetUp () override {
		mesh.name = "aligned_test_mesh";

		mesh.cpu_state.vertices.clear ();
		mesh.cpu_state.normals.clear ();
		mesh.cpu_state.indices.clear ();

		mesh.gpu_state.vertices.clear ();
		mesh.gpu_state.indices.clear ();
	}

	void MakeTriangle () {
		mesh.cpu_state.vertices = {
			{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}
		};

		mesh.cpu_state.normals = {
			{0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}
		};

		mesh.cpu_state.indices = {0, 1, 2};
	}
};

TEST_F (MeshTransferAlignedTest, RequiresVertices) {
	mesh.cpu_state.normals = {{0, 0, 1}};
	mesh.cpu_state.indices = {0};

	EXPECT_DEATH (MeshTransfer::to_gpu (mesh), ".*");
}

TEST_F (MeshTransferAlignedTest, RequiresNormals) {
	mesh.cpu_state.vertices = {{0, 0, 0}};
	mesh.cpu_state.indices = {0};

	EXPECT_DEATH (MeshTransfer::to_gpu (mesh), ".*");
}

TEST_F (MeshTransferAlignedTest, RequiresIndices) {
	mesh.cpu_state.vertices = {{0, 0, 0}};
	mesh.cpu_state.normals = {{0, 0, 1}};

	EXPECT_DEATH (MeshTransfer::to_gpu (mesh), ".*");
}

TEST_F (MeshTransferAlignedTest, RequiresVertexNormalCountMatch) {
	mesh.cpu_state.vertices = {{0, 0, 0}, {1, 0, 0}};
	mesh.cpu_state.normals = {{0, 0, 1}};
	mesh.cpu_state.indices = {0, 1};

	EXPECT_DEATH (MeshTransfer::to_gpu (mesh), ".*");
}

TEST_F (MeshTransferAlignedTest, CopiesIndicesCorrectly) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	ASSERT_EQ (mesh.gpu_state.indices.size (), mesh.cpu_state.indices.size ());
	EXPECT_EQ (mesh.gpu_state.indices, mesh.cpu_state.indices);
}

TEST_F (MeshTransferAlignedTest, CreatesCorrectVertexCount) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	EXPECT_EQ (
		mesh.gpu_state.vertices.size (), mesh.cpu_state.vertices.size ()
	);
}

TEST_F (MeshTransferAlignedTest, PacksPositionCorrectly) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	const Block& b = mesh.gpu_state.vertices[0];

	EXPECT_FLOAT_EQ (b.data[0], 0.f);
	EXPECT_FLOAT_EQ (b.data[1], 0.f);
	EXPECT_FLOAT_EQ (b.data[2], 0.f);
	EXPECT_FLOAT_EQ (b.data[3], 1.f);
}

TEST_F (MeshTransferAlignedTest, PacksNormalCorrectly) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	const Block& b = mesh.gpu_state.vertices[0];

	EXPECT_FLOAT_EQ (b.data[4], 0.f);
	EXPECT_FLOAT_EQ (b.data[5], 0.f);
	EXPECT_FLOAT_EQ (b.data[6], 1.f);
	EXPECT_FLOAT_EQ (b.data[7], 0.f);
}

TEST_F (MeshTransferAlignedTest, RebuildsGpuOnCpuChange) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	auto first = mesh.gpu_state.vertices;

	mesh.cpu_state.vertices[0] = {2.f, 3.f, 4.f};
	mesh.cpu_state.normals[0] = {1.f, 0.f, 0.f};

	MeshTransfer::to_gpu (mesh);

	const Block& b = mesh.gpu_state.vertices[0];

	EXPECT_NE (b.data[0], first[0].data[0]);
	EXPECT_NE (b.data[4], first[0].data[4]);
}

TEST_F (MeshTransferAlignedTest, CpuGpuCpuRoundTripPreservesData) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	mesh.cpu_state.vertices.clear ();
	mesh.cpu_state.normals.clear ();
	mesh.cpu_state.indices.clear ();

	MeshTransfer::to_cpu (mesh);

	ASSERT_EQ (mesh.cpu_state.vertices.size (), 3);
	ASSERT_EQ (mesh.cpu_state.normals.size (), 3);
	ASSERT_EQ (mesh.cpu_state.indices.size (), 3);

	EXPECT_FLOAT_EQ (mesh.cpu_state.vertices[0].x, 0.f);
	EXPECT_FLOAT_EQ (mesh.cpu_state.vertices[1].x, 1.f);
	EXPECT_FLOAT_EQ (mesh.cpu_state.vertices[2].y, 1.f);

	EXPECT_FLOAT_EQ (mesh.cpu_state.normals[0].z, 1.f);
}

TEST_F (MeshTransferAlignedTest, BlockAlignmentInvariant) {
	static_assert (sizeof (Block) % ALIGNMENT == 0);
	static_assert (alignof (Block) >= ALIGNMENT);
}

TEST_F (MeshTransferAlignedTest, GpuBlockAddressesAreAligned) {
	MakeTriangle ();
	MeshTransfer::to_gpu (mesh);

	for (const Block& b : mesh.gpu_state.vertices) {
		const auto addr = reinterpret_cast<uintptr_t> (&b);
		EXPECT_EQ (addr % ALIGNMENT, 0u);
	}
}
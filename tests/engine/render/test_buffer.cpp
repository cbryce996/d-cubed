#include <gtest/gtest.h>

#include "assets/mesh/mesh.h"
#include "render/buffers/buffer.h"
#include "render/drawable.h"

class BufferManagerTest : public ::testing::Test {
  protected:
	SDL_GPUDevice* fakeDevice = reinterpret_cast<SDL_GPUDevice*> (0x1);
	BufferManager* manager = nullptr;

	MeshInstance mesh;
	Drawable drawable;

	void SetUp () override {
		manager = new BufferManager (fakeDevice);

		mesh.name = "test_mesh";

		mesh.gpu_state.vertices.resize (3);
		mesh.gpu_state.indices = {0, 1, 2};

		drawable.mesh = &mesh;
		drawable.material = reinterpret_cast<MaterialInstance*> (0x1);

		drawable.instance_blocks.resize (2);
	}

	void TearDown () override { delete manager; }
};

TEST_F (BufferManagerTest, CreatesVertexBuffer) {
	Buffer* buf = manager->get_or_create_vertex_buffer (mesh);

	ASSERT_NE (buf, nullptr);
	EXPECT_EQ (buf->name, "test_mesh_vertex");
	EXPECT_GT (buf->size, 0u);
}

TEST_F (BufferManagerTest, VertexBufferIsCached) {
	const Buffer* b1 = manager->get_or_create_vertex_buffer (mesh);
	const Buffer* b2 = manager->get_or_create_vertex_buffer (mesh);

	EXPECT_EQ (b1->name, b2->name);
	EXPECT_EQ (b1->size, b2->size);
}

TEST_F (BufferManagerTest, CreatesIndexBuffer) {
	Buffer* buf = manager->get_or_create_index_buffer (mesh);

	ASSERT_NE (buf, nullptr);
	EXPECT_EQ (buf->name, "test_mesh_index");
	EXPECT_GT (buf->size, 0u);
}

TEST_F (BufferManagerTest, IndexBufferIsCached) {
	const Buffer* b1 = manager->get_or_create_index_buffer (mesh);
	const Buffer* b2 = manager->get_or_create_index_buffer (mesh);

	EXPECT_EQ (b1->name, b2->name);
	EXPECT_EQ (b1->size, b2->size);
}

TEST_F (BufferManagerTest, CreatesInstanceBuffer) {
	Buffer* buf = manager->get_or_create_instance_buffer (drawable);

	ASSERT_NE (buf, nullptr);
	EXPECT_GT (buf->size, 0u);
}

TEST_F (BufferManagerTest, InstanceBufferIsCached) {
	const Buffer* b1 = manager->get_or_create_instance_buffer (drawable);
	const Buffer* b2 = manager->get_or_create_instance_buffer (drawable);

	EXPECT_EQ (b1->name, b2->name);
	EXPECT_EQ (b1->size, b2->size);
}

TEST_F (BufferManagerTest, BufferRegistryStoresBuffers) {
	manager->get_or_create_vertex_buffer (mesh);
	manager->get_or_create_index_buffer (mesh);
	manager->get_or_create_instance_buffer (drawable);

	EXPECT_NE (manager->get_buffer ("test_mesh_vertex"), nullptr);
	EXPECT_NE (manager->get_buffer ("test_mesh_index"), nullptr);
}

TEST_F (BufferManagerTest, VertexBufferSizeMatchesData) {
	const Buffer* buf = manager->get_or_create_vertex_buffer (mesh);

	const size_t expected = mesh.gpu_state.vertices.size () * sizeof (Block);
	EXPECT_EQ (buf->size, expected);
}

TEST_F (BufferManagerTest, IndexBufferSizeMatchesData) {
	const Buffer* buf = manager->get_or_create_index_buffer (mesh);

	const size_t expected = mesh.gpu_state.indices.size () * sizeof (uint32_t);
	EXPECT_EQ (buf->size, expected);
}

TEST_F (BufferManagerTest, InstanceBufferSizeMatchesData) {
	const Buffer* buf = manager->get_or_create_instance_buffer (drawable);

	const size_t expected = drawable.instance_blocks.size () * sizeof (Block);
	EXPECT_EQ (buf->size, expected);
}

TEST_F (BufferManagerTest, BufferAlignmentInvariant) {
	static_assert (sizeof (Block) % ALIGNMENT == 0);
}

TEST_F (BufferManagerTest, WriteRejectsOversize) {
	Buffer* buf = manager->get_or_create_vertex_buffer (mesh);

	const std::vector<Block> huge (mesh.gpu_state.vertices.size () + 10);

	EXPECT_DEATH (
		manager->write (huge.data (), huge.size () * sizeof (Block), *buf), ".*"
	);
}

TEST_F (BufferManagerTest, WriteRequiresAlignedSize) {
	Buffer* buf = manager->get_or_create_vertex_buffer (mesh);

	constexpr char badData[3] = {1, 2, 3};

	EXPECT_DEATH (manager->write (badData, 3, *buf), ".*");
}

TEST_F (BufferManagerTest, BufferKeysAreUniquePerType) {
	const auto* v = manager->get_or_create_vertex_buffer (mesh);
	const auto* i = manager->get_or_create_index_buffer (mesh);

	EXPECT_NE (v->name, i->name);
}

TEST_F (BufferManagerTest, BufferReuseSameMesh) {
	const auto* v1 = manager->get_or_create_vertex_buffer (mesh);
	const auto* v2 = manager->get_or_create_vertex_buffer (mesh);

	EXPECT_EQ (v1->name, v2->name);
}

TEST_F (BufferManagerTest, BufferManagerDoesNotReturnUnknownBuffer) {
	EXPECT_EQ (manager->get_buffer ("nonexistent"), nullptr);
}

#include "memory.h"
#include "render/block.h"

#include <cstring>
#include <glm/glm.hpp>
#include <gtest/gtest.h>

class BlockTest : public ::testing::Test {
  protected:
	Block block = {};

	void SetUp () override { block.clear (); }
};

TEST_F (BlockTest, ClearZerosMemory) {
	for (float& f : block.data) {
		f = 123.456f;
	}

	block.clear ();

	for (float f : block.data) {
		EXPECT_EQ (f, 0.0f);
	}
}

TEST_F (BlockTest, WriteAndReadVec4) {
	constexpr glm::vec4 v (1.0f, 2.0f, 3.0f, 4.0f);

	block.write (0, v);

	const glm::vec4 out = block.to_cpu (0);

	EXPECT_EQ (out.x, 1.0f);
	EXPECT_EQ (out.y, 2.0f);
	EXPECT_EQ (out.z, 3.0f);
	EXPECT_EQ (out.w, 4.0f);
}

TEST_F (BlockTest, WriteMultipleSlots) {
	glm::vec4 a (1, 2, 3, 4);
	glm::vec4 b (5, 6, 7, 8);
	glm::vec4 c (9, 10, 11, 12);
	glm::vec4 d (13, 14, 15, 16);

	block.write (0, a);
	block.write (1, b);
	block.write (2, c);
	block.write (3, d);

	EXPECT_EQ (block.to_cpu (0), a);
	EXPECT_EQ (block.to_cpu (1), b);
	EXPECT_EQ (block.to_cpu (2), c);
	EXPECT_EQ (block.to_cpu (3), d);
}

TEST_F (BlockTest, FromVec4Factory) {
	constexpr glm::vec4 v (7, 8, 9, 10);

	const Block b = Block::from (v);

	const glm::vec4 out = b.to_cpu (0);

	EXPECT_EQ (out, v);

	for (size_t i = 1; i < BASE_COLLECTION_SIZE; ++i) {
		glm::vec4 zero = b.to_cpu (i);
		EXPECT_EQ (zero, glm::vec4 (0.0f));
	}
}

TEST_F (BlockTest, MemoryLayoutIsContiguous) {
	constexpr glm::vec4 v (1, 2, 3, 4);
	block.write (0, v);

	const float* raw = block.data;

	EXPECT_EQ (raw[0], 1.0f);
	EXPECT_EQ (raw[1], 2.0f);
	EXPECT_EQ (raw[2], 3.0f);
	EXPECT_EQ (raw[3], 4.0f);
}

TEST_F (BlockTest, AlignmentIsCorrect) {
	const uintptr_t addr = reinterpret_cast<uintptr_t> (&block);
	EXPECT_EQ (addr % UNIFORM_ALIGNMENT, 0u);
}

TEST_F (BlockTest, WriteOutOfBoundsSlotDeath) {
	constexpr glm::vec4 v (1, 2, 3, 4);
	EXPECT_DEATH (block.write (BASE_COLLECTION_SIZE, v), ".*");
}

TEST_F (BlockTest, ReadOutOfBoundsSlotDeath) {
	EXPECT_DEATH (block.to_cpu (BASE_COLLECTION_SIZE), ".*");
}

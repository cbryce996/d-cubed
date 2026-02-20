#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

#include "core/storage/maps/slot.h"

struct TestRecord {
	uint32_t a = 0;
	uint32_t b = 0;
};

class DenseSlotMapTest : public ::testing::Test {
  protected:
	DenseSlotMap map;
};

TEST_F (DenseSlotMapTest, AllocateReturnsValidHandleAndMemory) {
	const Handle handle = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	EXPECT_TRUE (handle.valid ());
	EXPECT_TRUE (map.valid (handle));

	void* pointer = map.try_get (handle);
	ASSERT_NE (pointer, nullptr);

	auto* record_1 = static_cast<TestRecord*> (pointer);
	record_1->a = 123;
	record_1->b = 456;

	auto* record_2 = static_cast<TestRecord*> (map.try_get (handle));
	ASSERT_NE (record_2, nullptr);
	EXPECT_EQ (record_2->a, 123u);
	EXPECT_EQ (record_2->b, 456u);
}

TEST_F (DenseSlotMapTest, FreeInvalidatesHandle) {
	const Handle handle = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	ASSERT_TRUE (map.valid (handle));

	EXPECT_TRUE (map.free (handle));
	EXPECT_FALSE (map.valid (handle));
	EXPECT_EQ (map.try_get (handle), nullptr);

	EXPECT_FALSE (map.free (handle));
}

TEST_F (DenseSlotMapTest, ReuseIdBumpsGeneration) {
	const Handle handle_1 = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	ASSERT_TRUE (map.valid (handle_1));

	const uint32_t old_id = handle_1.id;
	const uint32_t old_gen = handle_1.generation;

	ASSERT_TRUE (map.free (handle_1));

	const Handle handle_2 = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	ASSERT_TRUE (map.valid (handle_2));

	EXPECT_EQ (handle_2.id, old_id);

	EXPECT_NE (handle_2.generation, old_gen);
	EXPECT_FALSE (map.valid (handle_1));
	EXPECT_EQ (map.try_get (handle_1), nullptr);
}

TEST_F (DenseSlotMapTest, InvalidHandlesAreRejected) {
	Handle invalid{};
	invalid.id = 0xFFFFFFFFu;
	invalid.generation = 0;

	EXPECT_FALSE (map.valid (invalid));
	EXPECT_EQ (map.try_get (invalid), nullptr);
	EXPECT_FALSE (map.free (invalid));
}

TEST_F (DenseSlotMapTest, OutOfRangeIdIsInvalid) {
	Handle handle;
	handle.id = 999999u;
	handle.generation = 0;

	EXPECT_FALSE (map.valid (handle));
	EXPECT_EQ (map.try_get (handle), nullptr);
	EXPECT_FALSE (map.free (handle));
}

TEST_F (DenseSlotMapTest, WrongGenerationIsInvalid) {
	const Handle handle = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	ASSERT_TRUE (map.valid (handle));

	Handle wrong = handle;
	wrong.generation += 1;

	EXPECT_FALSE (map.valid (wrong));
	EXPECT_EQ (map.try_get (wrong), nullptr);
	EXPECT_FALSE (map.free (wrong));
}

TEST_F (DenseSlotMapTest, ClearResetsStorageAndInvalidatesHandles) {
	const Handle handle_1 = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	const Handle handle_2 = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	ASSERT_TRUE (map.valid (handle_1));
	ASSERT_TRUE (map.valid (handle_2));

	map.clear ();

	EXPECT_FALSE (map.valid (handle_1));
	EXPECT_FALSE (map.valid (handle_2));
	EXPECT_EQ (map.try_get (handle_1), nullptr);
	EXPECT_EQ (map.try_get (handle_2), nullptr);

	const Handle handle_3 = map.allocate (
		sizeof (TestRecord), alignof (TestRecord)
	);
	EXPECT_TRUE (map.valid (handle_3));
	EXPECT_NE (map.try_get (handle_3), nullptr);
}

TEST_F (DenseSlotMapTest, DeathOnDifferentRecordSizeOrAlign) {
	(void)map.allocate (sizeof (TestRecord), alignof (TestRecord));

	EXPECT_DEATH (
		{ (void)map.allocate (sizeof (TestRecord) + 4, alignof (TestRecord)); },
		".*"
	);
}

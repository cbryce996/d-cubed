#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "core/storage/policies/cache.h"
#include "core/storage/storage.h"
#include "fixtures.h"

class CacheTest : public ::testing::Test {
  protected:
	FakeStorage storage;
};

TEST_F (CacheTest, GetOrCreateReturnsSameHandleForEqualState) {
	const auto factory = std::make_shared<FakeFactory> ();
	Cache<FakeState, FakeFactory> cache (factory);

	const FakeState state_1{42};
	const FakeState state_2{42};

	const Handle handle_1 = cache.get_or_create (state_1);
	const Handle handle_2 = cache.get_or_create (state_2);

	EXPECT_EQ (handle_1.id, handle_2.id);
	EXPECT_EQ (factory->created_states.size (), 1u);
}

TEST_F (CacheTest, GetOrCreateCreatesNewHandleForDifferentState) {
	const auto factory = std::make_shared<FakeFactory> ();
	Cache<FakeState, FakeFactory> cache (factory);

	const Handle handle_1 = cache.get_or_create (FakeState{1});
	const Handle handle_2 = cache.get_or_create (FakeState{2});

	EXPECT_NE (handle_1.id, handle_2.id);
	EXPECT_EQ (factory->created_states.size (), 2u);
}

TEST_F (CacheTest, ClearDestroysAllCachedHandles) {
	const auto factory = std::make_shared<FakeFactory> ();
	Cache<FakeState, FakeFactory> cache (factory);

	const Handle handle_1 = cache.get_or_create (FakeState{1});
	const Handle handle_2 = cache.get_or_create (FakeState{2});

	cache.clear ();

	ASSERT_EQ (factory->destroyed_ids.size (), 2u);
	EXPECT_TRUE (
		(factory->destroyed_ids[0] == handle_1.id
		 || factory->destroyed_ids[1] == handle_1.id)
	);
	EXPECT_TRUE (
		(factory->destroyed_ids[0] == handle_2.id
		 || factory->destroyed_ids[1] == handle_2.id)
	);
}
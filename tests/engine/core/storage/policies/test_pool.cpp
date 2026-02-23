#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "../../../../../src/engine/core/storage/lifetime/pool.h"
#include "core/storage/storage.h"
#include "fixtures.h"

class PoolTest : public ::testing::Test {
  protected:
	FakeStorage storage;
};

TEST_F (PoolTest, AcquireCreatesWhenNoFreeHandle) {
	const auto factory = std::make_shared<FakeFactory> ();
	Pool<FakeState, FakeFactory> pool (*factory);

	const Handle handle = pool.acquire (FakeState{7});
	EXPECT_TRUE (handle.valid ());
	EXPECT_EQ (factory->created_states.size (), 1u);
}

TEST_F (PoolTest, ReleaseThenAcquireReusesHandleForSameState) {
	const auto factory = std::make_shared<FakeFactory> ();
	Pool<FakeState, FakeFactory> pool (*factory);

	const FakeState state{7};
	const Handle handle_1 = pool.acquire (state);
	pool.release (state, handle_1);

	const Handle handle_2 = pool.acquire (state);
	EXPECT_EQ (handle_1.id, handle_2.id);
	EXPECT_EQ (factory->created_states.size (), 1u);
}

TEST_F (PoolTest, DifferentStateDoesNotReuseOtherStatesHandle) {
	const auto factory = std::make_shared<FakeFactory> ();
	Pool<FakeState, FakeFactory> pool (*factory);

	const FakeState state_1{1};
	const FakeState state_2{2};

	const Handle handle_1 = pool.acquire (state_1);
	pool.release (state_1, handle_1);

	const Handle handle_2 = pool.acquire (state_2);

	EXPECT_NE (handle_1.id, handle_2.id);
	EXPECT_EQ (factory->created_states.size (), 2u);
}

TEST_F (PoolTest, ClearDestroysAllFreeHandles) {
	const auto factory = std::make_shared<FakeFactory> ();
	Pool<FakeState, FakeFactory> pool (*factory);

	const FakeState state{3};
	const Handle handle_1 = pool.acquire (state);
	const Handle handle_2 = pool.acquire (state);

	pool.release (state, handle_1);
	pool.release (state, handle_2);

	pool.clear ();

	EXPECT_EQ (factory->destroyed_ids.size (), 2u);
}
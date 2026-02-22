#include <cstdint>
#include <gtest/gtest.h>
#include <memory>

#include "core/storage/state.h"

struct State final : IState<State> {
	int a = 0;
	int b = 0;

	explicit State (const int a = 0, const int b = 0) : a (a), b (b) {}

	size_t hash () const override {
		return (static_cast<size_t> (a) * 1315423911u)
			   ^ static_cast<size_t> (b);
	}

	bool equals (const State& other) const override {
		return a == other.a && b == other.b;
	}

	std::unique_ptr<State> clone () const override {
		return std::make_unique<State> (*this);
	}
};

TEST (StateTest, CloneCreatesIndependentCopy) {
	const State state{1, 2};
	const auto state_clone = state.clone ();
	ASSERT_NE (state_clone, nullptr);
	EXPECT_TRUE (state.equals (*state_clone));
	EXPECT_EQ (state.hash (), state_clone->hash ());

	state_clone->a = 99;
	EXPECT_FALSE (state.equals (*state_clone));
}

TEST (StateKeyTest, HashAndEqualsReflectUnderlyingState) {
	const State state_1{1, 2};
	const State state_2{1, 2};
	const State state_3{2, 3};

	const IStateKey state_key_1{state_1};
	const IStateKey state_key_2{state_2};
	const IStateKey state_key_3{state_3};

	constexpr IStateKey<State>::Hash hasher;
	constexpr IStateKey<State>::Equals equals;

	EXPECT_TRUE (equals (state_key_1, state_key_2));
	EXPECT_FALSE (equals (state_key_1, state_key_3));

	EXPECT_EQ (hasher (state_key_1), hasher (state_key_2));
	EXPECT_NE (hasher (state_key_1), hasher (state_key_3));
}

TEST (StateKeyTest, MoveOnlyWorks) {
	const State state{5, 6};
	IStateKey state_key_1{state};

	const IStateKey state_key_2{std::move (state_key_1)};
	constexpr IStateKey<State>::Hash hasher;

	EXPECT_EQ (hasher (state_key_2), state.hash ());
}
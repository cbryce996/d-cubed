#include "render/material.h"

#include <gtest/gtest.h>

class MaterialStateTest : public ::testing::Test {
  protected:
	MaterialState base_state{};

	void SetUp () override {
		base_state = {
			.shader = "test_shader",
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.cull_mode = SDL_GPU_CULLMODE_BACK,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.enable_depth_test = true,
			.enable_depth_write = true
		};
	}
};

TEST_F (MaterialStateTest, EqualityOperatorTrueForSameState) {
	const MaterialState copy = base_state;
	EXPECT_TRUE (base_state == copy);
}

TEST_F (MaterialStateTest, EqualityOperatorFalseForDifferentShader) {
	MaterialState other = base_state;
	other.shader = "other_shader";

	EXPECT_FALSE (base_state == other);
}

TEST_F (MaterialStateTest, EqualityOperatorFalseForDifferentPrimitiveType) {
	MaterialState other = base_state;
	other.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

	EXPECT_FALSE (base_state == other);
}

TEST_F (MaterialStateTest, EqualityOperatorFalseForDifferentCullMode) {
	MaterialState other = base_state;
	other.cull_mode = SDL_GPU_CULLMODE_NONE;

	EXPECT_FALSE (base_state == other);
}

TEST_F (MaterialStateTest, EqualityOperatorFalseForDifferentDepthFlags) {
	MaterialState other = base_state;
	other.enable_depth_write = false;

	EXPECT_FALSE (base_state == other);
}

TEST_F (MaterialStateTest, HashIsStableForSameState) {
	constexpr std::hash<MaterialState> hasher;

	const size_t h1 = hasher (base_state);
	const size_t h2 = hasher (base_state);

	EXPECT_EQ (h1, h2);
}

TEST_F (MaterialStateTest, EquivalentStatesProduceSameHash) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState copy = base_state;

	EXPECT_EQ (hasher (base_state), hasher (copy));
}

TEST_F (MaterialStateTest, DifferentShaderProducesDifferentHash) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState other = base_state;
	other.shader = "other_shader";

	EXPECT_NE (hasher (base_state), hasher (other));
}

TEST_F (MaterialStateTest, DifferentPrimitiveTypeProducesDifferentHash) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState other = base_state;
	other.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;

	EXPECT_NE (hasher (base_state), hasher (other));
}

TEST_F (MaterialStateTest, DifferentCullModeProducesDifferentHash) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState other = base_state;
	other.cull_mode = SDL_GPU_CULLMODE_NONE;

	EXPECT_NE (hasher (base_state), hasher (other));
}

TEST_F (MaterialStateTest, CompareOpIgnoredWhenDepthTestDisabled) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState a = base_state;
	a.enable_depth_test = false;
	a.compare_op = SDL_GPU_COMPAREOP_LESS;

	MaterialState b = base_state;
	b.enable_depth_test = false;
	b.compare_op = SDL_GPU_COMPAREOP_GREATER;

	EXPECT_EQ (hasher (a), hasher (b));
	EXPECT_TRUE (a == b);
}

TEST_F (MaterialStateTest, CompareOpAffectsHashWhenDepthTestEnabled) {
	constexpr std::hash<MaterialState> hasher;

	MaterialState a = base_state;
	a.compare_op = SDL_GPU_COMPAREOP_LESS;

	MaterialState b = base_state;
	b.compare_op = SDL_GPU_COMPAREOP_GREATER;

	EXPECT_NE (hasher (a), hasher (b));
	EXPECT_FALSE (a == b);
}

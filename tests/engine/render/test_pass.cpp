#include <gtest/gtest.h>

#include "render/pass/pass.h"

class RenderPassStateTest : public ::testing::Test {
  protected:
	RenderPassState base_state{};

	void SetUp () override {
		base_state = {
			.depth_compare = CompareOp::Less,
			.depth_format = TextureFormat::D32F,
			.depth_stencil_format = TextureFormat::D32F,
			.color_formats = {TextureFormat::BGRA8, TextureFormat::RGBA16F},
			.has_depth_stencil_target = true
		};
	}
};

TEST_F (RenderPassStateTest, EqualityOperatorTrueForSameState) {
	const RenderPassState copy = base_state;
	EXPECT_TRUE (base_state == copy);
}

TEST_F (RenderPassStateTest, EqualityOperatorFalseForDifferentDepthFormat) {
	RenderPassState other = base_state;
	other.depth_format = TextureFormat::BGRA8;

	EXPECT_FALSE (base_state == other);
}

TEST_F (RenderPassStateTest, EqualityOperatorFalseForDifferentColorFormats) {
	RenderPassState other = base_state;
	other.color_formats.push_back (TextureFormat::RGBA8);

	EXPECT_FALSE (base_state == other);
}

TEST_F (RenderPassStateTest, HashIsStableForSameState) {
	constexpr std::hash<RenderPassState> hasher;

	const size_t h1 = hasher (base_state);
	const size_t h2 = hasher (base_state);

	EXPECT_EQ (h1, h2);
}

TEST_F (RenderPassStateTest, EquivalentStatesProduceSameHash) {
	constexpr std::hash<RenderPassState> hasher;

	const RenderPassState copy = base_state;

	EXPECT_EQ (hasher (base_state), hasher (copy));
}

TEST_F (RenderPassStateTest, DifferentStatesProduceDifferentHash) {
	constexpr std::hash<RenderPassState> hasher;

	RenderPassState other = base_state;
	other.has_depth_stencil_target = false;

	EXPECT_NE (hasher (base_state), hasher (other));
}

TEST_F (RenderPassStateTest, ColorFormatOrderAffectsHash) {
	constexpr std::hash<RenderPassState> hasher;

	RenderPassState reordered = base_state;
	std::ranges::reverse (reordered.color_formats);

	EXPECT_NE (hasher (base_state), hasher (reordered));
}

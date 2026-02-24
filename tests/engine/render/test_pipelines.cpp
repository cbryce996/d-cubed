#include "render/graph/graph.h"
#include "render/material.h"
#include "render/pipelines/pipeline.h"

#include <gtest/gtest.h>

class FakePipelineFactory final : public IPipelineFactory {
  public:
	int create_count = 0;

	Pipeline* create_pipeline (const PipelineState& state) override {
		++create_count;

		auto* pipeline = new Pipeline{};
		pipeline->pipeline = reinterpret_cast<SDL_GPUGraphicsPipeline*> (
			0xDEADBEEF
		);

		pipeline->name = state.material_state.vertex_shader + "_"
						 + state.material_state.fragment_shader;

		return pipeline;
	}
};

class PipelineManagerTest : public ::testing::Test {
  protected:
	RenderPassState render_pass_state{};
	MaterialState material_state{};
	PipelineState pipeline_state{};

	void SetUp () override {
		render_pass_state = {
			.depth_compare = CompareOp::Less,
			.depth_format = TextureFormat::D32F,
			.depth_stencil_format = TextureFormat::D32F,
			.color_formats = {TextureFormat::BGRA8},
			.has_depth_stencil_target = false
		};

		material_state = {
			.vertex_shader = "vertex_shader",
			.fragment_shader = "fragment_shader",
			.primitive_type = PrimitiveType::TriangleList,
			.cull_mode = CullMode::Back,
			.compare_op = CompareOp::Less,
			.enable_depth_test = true,
			.enable_depth_write = true
		};

		pipeline_state = {
			.render_pass_state = render_pass_state,
			.material_state = material_state,
		};
	}
};

TEST_F (PipelineManagerTest, CreatesPipelineIfCacheMiss) {
	const auto factory = std::make_shared<FakePipelineFactory> ();
	PipelineManager pipeline_manager (factory);

	Pipeline* pipeline = pipeline_manager.get_or_create (pipeline_state);

	ASSERT_NE (pipeline, nullptr);
	EXPECT_EQ (factory->create_count, 1);
}

TEST_F (PipelineManagerTest, ReturnsSamePipelineOnCacheHit) {
	const auto factory = std::make_shared<FakePipelineFactory> ();
	PipelineManager pipeline_manager (factory);

	Pipeline* first = pipeline_manager.get_or_create (pipeline_state);

	Pipeline* second = pipeline_manager.get_or_create (pipeline_state);

	EXPECT_EQ (first, second);
	EXPECT_EQ (factory->create_count, 1);
}

TEST_F (PipelineManagerTest, PipelineStateHashIsStableForSameState) {
	constexpr std::hash<PipelineState> hasher;

	const size_t h1 = hasher (pipeline_state);
	const size_t h2 = hasher (pipeline_state);

	EXPECT_EQ (h1, h2);
}

TEST_F (PipelineManagerTest, EquivalentStatesProduceSameHash) {
	constexpr std::hash<PipelineState> hasher;

	RenderPassState pass_copy = render_pass_state;
	MaterialState mat_copy = material_state;

	const PipelineState equivalent{
		.render_pass_state = pass_copy,
		.material_state = mat_copy,
	};

	EXPECT_EQ (hasher (pipeline_state), hasher (equivalent));
}

TEST_F (PipelineManagerTest, DifferentMaterialStateIsNotEqual) {
	MaterialState different = material_state;
	different.vertex_shader = "other_vertex_shader";
	different.fragment_shader = "other_fragment_shader";

	const PipelineState other{
		.render_pass_state = render_pass_state,
		.material_state = different,
	};

	EXPECT_FALSE (pipeline_state == other);
}

TEST_F (PipelineManagerTest, DifferentStateCreatesDifferentPipeline) {
	const auto factory = std::make_shared<FakePipelineFactory> ();
	PipelineManager pipeline_manager (factory);

	Pipeline* first = pipeline_manager.get_or_create (pipeline_state);

	MaterialState different = material_state;
	different.vertex_shader = "other_vertex_shader";
	different.fragment_shader = "other_fragment_shader";

	const PipelineState other{
		.render_pass_state = render_pass_state,
		.material_state = different,
	};

	Pipeline* second = pipeline_manager.get_or_create (other);

	EXPECT_NE (first, second);
	EXPECT_EQ (factory->create_count, 2);
}
#ifndef PIPELINE_H
#define PIPELINE_H

#include "render/material.h"
#include "render/pass/pass.h"

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

class BufferManager;
struct UniformBinding;
struct RenderPassState;
struct MaterialState;

constexpr SDL_GPUVertexElementFormat get_format () {
#if BASE_COLLECTION_SIZE == 4
	return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
#else
	return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
#endif
};

struct Pipeline {
	SDL_GPUGraphicsPipeline* pipeline = nullptr;
	std::string name;
};

struct PipelineState {
	RenderPassState render_pass_state;
	MaterialState material_state;

	bool operator== (const PipelineState& other) const;
};

template <> struct std::hash<PipelineState> {
	size_t operator() (const PipelineState& pipeline_state) const noexcept {
		size_t h = 0;

		hash_combine (
			h, std::hash<RenderPassState> () (pipeline_state.render_pass_state)
		);
		hash_combine (
			h, std::hash<MaterialState> () (pipeline_state.material_state)
		);

		return h;
	}
};

class IPipelineFactory {
  public:
	virtual ~IPipelineFactory () = default;

	virtual Pipeline* create_pipeline (const PipelineState& pipeline_state) = 0;
};

class PipelineManager {
  public:
	explicit PipelineManager (const std::shared_ptr<IPipelineFactory>& factory);
	~PipelineManager ();

	Pipeline* get_or_create (const PipelineState& state);
	void push_uniforms (
		const std::vector<UniformBinding>& uniform_bindings,
		const BufferManager& buffer_manager
	) const;

  private:
	std::shared_ptr<IPipelineFactory> factory;
	std::unordered_map<PipelineState, Pipeline> pipelines;
};

#endif // PIPELINE_H
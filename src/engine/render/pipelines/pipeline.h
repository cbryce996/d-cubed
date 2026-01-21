#ifndef PIPELINE_H
#define PIPELINE_H

#include "render/material.h"
#include "render/pass.h"
#include "render/shaders/shader.h"

#include <functional>
#include <string>
#include <unordered_map>

struct RenderPassState;
struct MaterialState;
struct MeshState;

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
	const RenderPassState* render_pass_state;
	const MaterialState* material_state;

	bool operator== (const PipelineState& other) const {
		return *render_pass_state == *other.render_pass_state
			   && *material_state == *other.material_state;
	}
};

namespace std {

template <> struct hash<PipelineState> {
	size_t operator() (const PipelineState& pipeline_state) const noexcept {
		size_t h = 0;

		hash_combine (
			h, std::hash<RenderPassState> () (*pipeline_state.render_pass_state)
		);
		hash_combine (
			h, std::hash<MaterialState> () (*pipeline_state.material_state)
		);

		return h;
	}
};

}

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

  private:
	std::shared_ptr<IPipelineFactory> factory;
	std::unordered_map<PipelineState, Pipeline> pipelines;
};

#endif // PIPELINE_H
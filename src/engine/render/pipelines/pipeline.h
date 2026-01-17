#ifndef PIPELINE_H
#define PIPELINE_H

#include "render/graph/graph.h"
#include "render/material.h"
#include "render/shaders/shader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <string>
#include <unordered_map>

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

class PipelineManager {
  public:
	explicit PipelineManager (
		SDL_GPUDevice* device, SDL_Window* window,
		const std::shared_ptr<ShaderManager>& shader_manager
	);
	~PipelineManager ();
	Pipeline*
	get_or_create (
		const RenderPassLayout& render_pass_layout, Material& material
	);

	Pipeline* get_pipeline (const RenderPassLayout& render_pass_layout);
	void
	add_pipeline (
		const RenderPassLayout& render_pass_layout, const Pipeline& pipeline
	);

	[[nodiscard]] SDL_GPUGraphicsPipeline*
	create_pipeline (
		const RenderPassLayout& render_pass_layout, Material& material
	) const;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;

	std::shared_ptr<ShaderManager> shader_manager = nullptr;

	std::unordered_map<std::string, Pipeline> pipelines;
};

#endif // PIPELINE_H

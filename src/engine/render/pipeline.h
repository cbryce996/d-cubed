#ifndef PIPELINE_H
#define PIPELINE_H

#include "drawable.h"
#include "memory.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <string>
#include <unordered_map>

#include "shader.h"

constexpr SDL_GPUVertexElementFormat get_format () {
#if BASE_COLLECTION_SIZE == 4
	return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
#else
	return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
#endif
}

struct PipelineConfig {
	const Shader* shader;

	const SDL_GPUPrimitiveType primitive_type;
	const SDL_GPUCullMode cull_mode;
	const SDL_GPUCompareOp depth_compare;
	const SDL_GPUTextureFormat depth_format;
	const SDL_GPUCompareOp compare_op;
	const SDL_GPUTextureFormat depth_stencil_format;

	const bool enable_depth_test;
	const bool enable_depth_write;
	const bool has_depth_stencil_target;

	std::string key () const {
		return shader->name + "_" + std::to_string (primitive_type) + "_"
			   + std::to_string (enable_depth_test);
	}
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
	void load_pipeline (
		const PipelineConfig* pipeline_config, const std::string& name
	);

	Pipeline* get_pipeline (const std::string& name);

	void add_pipeline (Pipeline& pipeline);

	SDL_GPUGraphicsPipeline*
	create_pipeline (const PipelineConfig& pipeline_config) const;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;

	std::shared_ptr<ShaderManager> shader_manager = nullptr;

	std::unordered_map<std::string, Pipeline> pipelines;
};

#endif // PIPELINE_H

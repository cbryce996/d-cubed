#ifndef PIPELINE_H
#define PIPELINE_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <string>
#include <unordered_map>

#include "drawable.h"
#include "shader.h"

struct PipelineConfig {
	std::string name;
	Shader* shader = nullptr;
	SDL_GPUVertexInputState vertex_input;
	SDL_GPUPrimitiveType primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	SDL_GPUCullMode cull_mode = SDL_GPU_CULLMODE_BACK;
	SDL_GPUCompareOp depth_compare = SDL_GPU_COMPAREOP_LESS;
	SDL_GPUTextureFormat depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	SDL_GPUCompareOp compare_op = SDL_GPU_COMPAREOP_LESS;
	bool enable_depth_test = true;
	bool enable_depth_write = true;
	bool has_depth = true;
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

	Pipeline* get_pipeline (const std::string& name);
	Pipeline* get_or_create_pipeline (const Drawable* drawable);

	void add_pipeline (Pipeline& pipeline);

	SDL_GPUGraphicsPipeline* create_pipeline (PipelineConfig& config) const;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;

	std::shared_ptr<ShaderManager> shader_manager = nullptr;

	std::unordered_map<std::string, Pipeline> pipelines;
};

#endif // PIPELINE_H

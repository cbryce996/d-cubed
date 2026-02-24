#ifndef FACTORY_H
#define FACTORY_H

#include "render/pipelines/pipeline.h"

#include <SDL3/SDL.h>

#include "render/shaders/shader.h"

struct Pipeline;
struct PipelineState;
class ShaderManager;

class SDLPipelineFactory : public IPipelineFactory {
  public:
	SDLPipelineFactory (
		SDL_GPUDevice* device, SDL_Window* window,
		const std::shared_ptr<ShaderManager>& shader_manager
	);
	~SDLPipelineFactory () override;

	SDL_GPUVertexInputRate to_sdl_input_rate (InputRate input_rate);
	SDL_GPUVertexElementFormat to_sdl_vertex_format (DataTypes data_type);

	SDL_GPUTextureFormat to_sdl_texture_format (TextureFormat format);

	SDL_GPUCompareOp to_sdl_compare_op (CompareOp op);

	SDL_GPUCullMode to_sdl_cull_mode (CullMode mode);

	SDL_GPUPrimitiveType to_sdl_primitive_type (PrimitiveType type);

	Pipeline* create_pipeline (const PipelineState& pipeline_state) override;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	std::shared_ptr<ShaderManager> shader_manager;
};

#endif // FACTORY_H

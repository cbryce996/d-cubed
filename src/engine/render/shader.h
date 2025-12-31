#ifndef SHADER_H
#define SHADER_H

#include "memory.h"

#include <string>
#include <unordered_map>

#include "SDL3/SDL_gpu.h"

struct ShaderConfig {
	Uint32 num_samplers = 0;
	Uint32 num_storage_textures = 0;
	Uint32 num_storage_buffers = 0;
	Uint32 num_uniform_buffers = 2;

	Uint32 props = 0;
	SDL_GPUShaderFormat format;
	SDL_GPUShaderStage stage;
	std::string entrypoint;
	std::string path;
};

struct Shader {
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	std::string name;

	uint32_t required_vertex_attributes = 4;
	uint32_t required_instance_attributes = 4;

	uint32_t block_size = sizeof(Block);
	uint32_t vertex_stride = required_vertex_attributes * block_size;
	uint32_t instance_stride = required_instance_attributes * block_size;
};

class ShaderManager {
  public:
	explicit ShaderManager (SDL_GPUDevice* device);
	~ShaderManager ();

	void load_shader (
		const ShaderConfig& vertex_shader, const ShaderConfig& fragment_shader,
		const std::string& name
	);
	void add_shader (Shader& shader);
	Shader* get_shader (const std::string& name);

  private:
	[[nodiscard]] SDL_GPUShader*
	compile_shader (const ShaderConfig& shader_config) const;

	SDL_GPUDevice* device = nullptr;
	std::unordered_map<std::string, Shader> shaders;
};

#endif // SHADER_H

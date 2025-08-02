#include "shader.h"

#include "SDL3/SDL_log.h"

ShaderManager::ShaderManager(SDL_GPUDevice* device) : device(device) {
	load_shader(
		ShaderConfig{
			.path = "../shaders/msl/cube_metal_uniform_lighting.metal",
			.entrypoint = "vert_main",
			.format = SDL_GPU_SHADERFORMAT_MSL,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 1
		},
		ShaderConfig{
			.path = "../shaders/msl/cube_metal_uniform_lighting.metal",
			.entrypoint = "frag_main",
			.format = SDL_GPU_SHADERFORMAT_MSL,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 1
		},
		"lit"
	);
}

ShaderManager::~ShaderManager() = default;

void ShaderManager::load_shader(
	const ShaderConfig& vertex_shader,
	const ShaderConfig& fragment_shader,
	const std::string& name
) {
	Shader shader{
		.vertex_shader = compile_shader(vertex_shader),
		.fragment_shader = compile_shader(fragment_shader),
		.name = name
	};
	add_shader(shader);
}

Shader* ShaderManager::get_shader(const std::string& name) {
	Shader* shader = shaders.contains(name) ? &shaders[name] : nullptr;
	if (!shader) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader not found.");
	}
	return shader;
}

void ShaderManager::add_shader(Shader& shader) {
	shaders.emplace(shader.name, shader);
}

SDL_GPUShader* ShaderManager::compile_shader(const ShaderConfig& shader_config) const {
	SDL_IOStream* file = SDL_IOFromFile(shader_config.path.c_str(), "r");
	if (!file) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to open shader: %s", shader_config.path.c_str());
		SDL_CloseIO(file);
		return nullptr;
	}

	const Sint64 size = SDL_GetIOSize(file);
	if (size <= 0) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader file has invalid size: %s", shader_config.path.c_str());
		SDL_CloseIO(file);
		return nullptr;
	}

	std::string source(size, '\0');
	const Sint64 read = SDL_ReadIO(file, source.data(), size);
	if (read != size) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to read entire shader file: %s", shader_config.path.c_str());
		SDL_CloseIO(file);
		return nullptr;
	}

	SDL_CloseIO(file);

	const SDL_GPUShaderCreateInfo info = {
		.code_size = static_cast<size_t>(size),
		.code = reinterpret_cast<const Uint8*>(source.data()),
		.entrypoint = shader_config.entrypoint.c_str(),
		.format = shader_config.format,
		.stage = shader_config.stage,
		.num_samplers = shader_config.num_samplers,
		.num_storage_textures = shader_config.num_storage_textures,
		.num_storage_buffers = shader_config.num_storage_buffers,
		.num_uniform_buffers = shader_config.num_uniform_buffers,
		.props = shader_config.props
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
	if (!shader) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader compilation failed.");
	}
	return shader;
}
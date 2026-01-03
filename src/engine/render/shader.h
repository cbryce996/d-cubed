#ifndef SHADER_H
#define SHADER_H

#include "memory.h"

#include <string>
#include <unordered_map>

#include "SDL3/SDL_gpu.h"

#include <map>

enum class ShaderDataType : uint32_t {
	None = 0,
	Float = 4,
	Vec2 = 8,
	Vec3 = 12,
	Vec4 = 16,
	Mat4 = 64,
	Int = 4
};

static const std::unordered_map<std::string, ShaderDataType> SHADER_TYPE_LOOKUP
	= {{"float", ShaderDataType::Float}, {"vec2", ShaderDataType::Vec2},
	   {"vec3", ShaderDataType::Vec3},	 {"vec4", ShaderDataType::Vec4},
	   {"mat4", ShaderDataType::Mat4},	 {"int", ShaderDataType::Int}};

namespace ShaderTypeUtils {
inline uint32_t get_size (ShaderDataType type) {
	return static_cast<uint32_t> (type);
}

inline SDL_GPUVertexElementFormat get_sdl_format (ShaderDataType type) {
	switch (type) {
	case ShaderDataType::Float:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
	case ShaderDataType::Vec2:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	case ShaderDataType::Vec3:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	case ShaderDataType::Vec4:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	default:
		return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
	}
}
}

struct UniformMember {
	std::string name;
	uint32_t offset;
	uint32_t size;
	ShaderDataType type;
};

struct UniformBlock {
	std::string name;
	uint32_t binding;
	uint32_t total_size;
	std::unordered_map<std::string, UniformMember> members;
};

struct ShaderConfig {
	uint32_t num_samplers = 0;
	uint32_t num_storage_textures = 0;
	uint32_t num_storage_buffers = 0;
	uint32_t num_uniform_buffers = 2;

	uint32_t props = 0;
	SDL_GPUShaderFormat format;
	SDL_GPUShaderStage stage;
	std::string entrypoint;
	std::string path;
};

struct ShaderSampler {
	std::string name;
	uint32_t set;
	uint32_t binding;
};

struct VertexAttribute {
	std::string name;
	uint32_t location;
	ShaderDataType type;
};

struct VertexBufferLayout {
	uint32_t stride = 0;
	SDL_GPUVertexInputRate input_rate;

	std::vector<VertexAttribute> attributes;
	std::vector<SDL_GPUVertexAttribute> sdl_attributes;
};

struct Shader {
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;
	std::string name;

	std::unordered_map<uint32_t, VertexBufferLayout> vertex_buffer_layouts;
	std::unordered_map<std::string, UniformBlock> uniform_blocks;

	std::vector<ShaderSampler> samplers;
};

class ShaderManager {
  public:
	explicit ShaderManager (SDL_GPUDevice* device);
	~ShaderManager ();
	std::vector<ShaderSampler> load_samplers (const std::string& json_path);

	bool load_shader (
		const ShaderConfig& vertex_config, const ShaderConfig& fragment_config,
		const std::string& name
	);

	Shader* get_shader (const std::string& name);

	void add_shader (const Shader& shader);

	static std::vector<VertexAttribute>
	load_vertex_attributes (const std::string& json_path);
	static std::unordered_map<std::string, UniformBlock>
	load_uniform_blocks (const std::string& json_path);

  private:
	[[nodiscard]] SDL_GPUShader*
	compile_shader (const ShaderConfig& config) const;

	SDL_GPUDevice* device = nullptr;
	std::unordered_map<std::string, Shader> shaders;
};

#endif // SHADER_H

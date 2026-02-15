#ifndef SHADER_H
#define SHADER_H

#include "utils.h"

#include <string>
#include <unordered_map>

#include "SDL3/SDL_gpu.h"

enum class DataTypes : uint8_t { None = 0, Float, Vec2, Vec3, Vec4, Mat4, Int };

enum class InputRate { PerVertex, PerInstance };

struct StorageTexture {
	std::string name;
	uint32_t set;
	uint32_t binding;
};

struct SampledTexture {
	std::string name;
	uint32_t set;
	uint32_t binding;
};

struct UniformBufferField {
	std::string name;
	uint32_t offset;
	uint32_t size;
	DataTypes type;
};

struct UniformBuffer {
	std::string name;
	uint32_t set;
	uint32_t binding;
	uint32_t total_size;
	std::unordered_map<std::string, UniformBufferField> fields;
};

struct VertexBufferField {
	std::string name;
	uint32_t location;
	DataTypes type;
	uint32_t offset = 0;
};

struct VertexBuffer {
	uint32_t stride = 0;
	InputRate input_rate;
	std::vector<VertexBufferField> fields;
};

struct Shader {
	std::string name;
	std::string entrypoint;
	std::string path;

	SDL_GPUShaderFormat format;
	SDL_GPUShaderStage stage;

	SDL_GPUShader* shader = nullptr;

	std::unordered_map<uint32_t, VertexBuffer> vertex_buffers;
	std::unordered_map<std::string, UniformBuffer> uniform_buffers;
	std::vector<SampledTexture> sampled_textures;
	std::vector<StorageTexture> storage_textures;

	[[nodiscard]] std::string json_path () const {
		std::string p = path;
		size_t last_dot = p.find_last_of ('.');
		if (last_dot != std::string::npos) {
			p = p.substr (0, last_dot) + ".json";
		}
		return p;
	}
};

class ShaderManager {
  public:
	explicit ShaderManager (SDL_GPUDevice* device);
	~ShaderManager ();

	std::optional<DataTypes> from_string (std::string_view string);
	uint32_t size_bytes (DataTypes t);

	bool load_shader (Shader& vertex_shader, Shader& fragment_shader);

	Shader* get_shader (const std::string& name);

	void add_shader (const Shader& shader);

	void load_vertex_buffers (Shader& shader);
	void load_uniform_buffers (Shader& shader);
	void load_sampled_textures (Shader& shader);

  private:
	void compile_shader (Shader& shader) const;

	SDL_GPUDevice* device = nullptr;
	std::unordered_map<std::string, Shader> shaders;
};

#endif // SHADER_H

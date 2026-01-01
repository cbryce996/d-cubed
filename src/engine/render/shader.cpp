#include "shader.h"

#include "SDL3/SDL_log.h"

#include <fstream>
#include <nlohmann/json.hpp>

ShaderManager::ShaderManager (SDL_GPUDevice* device) : device (device) {}

ShaderManager::~ShaderManager () = default;

std::vector<VertexAttribute>
ShaderManager::load_vertex_attributes (const std::string& json_path) {
	std::ifstream file (json_path);
	if (!file.is_open ()) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR, "Failed to open shader metadata: '%s'.",
			json_path.c_str ()
		);
		return {};
	}

	SDL_LogMessage (
		SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO,
		"Loading vertex attributes from: '%s'.", json_path.c_str ()
	);

	nlohmann::json data = nlohmann::json::parse (file);
	std::vector<VertexAttribute> attribute_info;

	if (data.contains ("inputs")) {
		for (const auto& input : data["inputs"]) {
			std::string json_type = input["type"];

			auto it = SHADER_TYPE_LOOKUP.find (json_type);
			if (it != SHADER_TYPE_LOOKUP.end ()) {
				VertexAttribute info;
				info.name = input.value ("name.", "unknown");
				info.location = input.value ("location.", 0);
				info.type = it->second;

				attribute_info.push_back (info);
			} else {
				SDL_LogError (
					SDL_LOG_CATEGORY_ERROR,
					"Warning: Unsupported shader input type '%s' in %s.",
					json_type.c_str (), json_path.c_str ()
				);
				return {};
			}
		}
	}

	return attribute_info;
}

std::unordered_map<std::string, UniformBlock>
ShaderManager::load_uniform_blocks (const std::string& json_path) {
	std::ifstream file (json_path);
	if (!file.is_open ()) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR, "Failed to open shader metadata: '%s'.",
			json_path.c_str ()
		);
		return {};
	}

	SDL_LogMessage (
		SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO,
		"Loading uniform blocks from: '%s'.", json_path.c_str ()
	);

	nlohmann::json data = nlohmann::json::parse (file);
	std::unordered_map<std::string, UniformBlock> uniform_blocks;

	if (data.contains ("ubos")) {
		for (const auto& ubo : data["ubos"]) {
			UniformBlock block;
			block.name = ubo["name"];
			block.binding = ubo["binding"];
			block.total_size = ubo["block_size"];

			std::string type_id = ubo["type"];
			auto members_json = data["types"][type_id]["members"];

			for (const auto& m : members_json) {
				UniformMember member;
				member.name = m["name"];
				member.offset = m["offset"];

				std::string m_type = m["type"];
				if (SHADER_TYPE_LOOKUP.count (m_type)) {
					member.type = SHADER_TYPE_LOOKUP.at (m_type);
					member.size = ShaderTypeUtils::get_size (member.type);
				}
				block.members[member.name] = member;
			}
			uniform_blocks[block.name] = block;
		}
	}

	return uniform_blocks;
}

void ShaderManager::load_shader (
	const ShaderConfig& v_config, const ShaderConfig& f_config,
	const std::string& name
) {
	if (get_shader (name)) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER, "Shader '%s' already loaded. Skipping.",
			name.c_str ()
		);
		return;
	}

	SDL_LogMessage (
		SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO, "Loading shader: '%s'.",
		name.c_str ()
	);

	Shader shader;
	shader.name = name;

	shader.vertex_shader = compile_shader (v_config);
	shader.fragment_shader = compile_shader (f_config);

	std::string json_path = v_config.path + ".json";

	shader.uniform_blocks = load_uniform_blocks (json_path);
	if (shader.uniform_blocks.empty ()) {
		SDL_LogCritical (
			SDL_LOG_CATEGORY_RENDER,
			"Aborting load for shader '%s' due to missing/invalid uniform "
			"metadata.",
			name.c_str ()
		);
		return;
	}

	std::vector<VertexAttribute> vertex_attributes = load_vertex_attributes (
		json_path
	);
	if (vertex_attributes.empty ()) {
		SDL_LogCritical (
			SDL_LOG_CATEGORY_RENDER,
			"Aborting load for shader '%s' due to missing/invalid vertex "
			"metadata.",
			name.c_str ()
		);
		return;
	}

	std::vector<SDL_GPUVertexAttribute> sdl_attributes;

	for (const auto& attribute : vertex_attributes) {
		uint32_t slot = (attribute.location == 0) ? 0 : 1;

		if (!shader.vertex_buffer_layouts.contains (slot)) {
			shader.vertex_buffer_layouts[slot].input_rate
				= (slot == 0) ? SDL_GPU_VERTEXINPUTRATE_VERTEX
							  : SDL_GPU_VERTEXINPUTRATE_INSTANCE;
		}

		VertexBufferLayout& layout = shader.vertex_buffer_layouts[slot];
		layout.attributes.push_back (attribute);

		// TODO: Extract into map
		uint32_t num_slots = (attribute.type == ShaderDataType::Mat4) ? 4 : 1;
		uint32_t bytes_per_slot = (attribute.type == ShaderDataType::Mat4)
									  ? 16
									  : ShaderTypeUtils::get_size (
											attribute.type
										);

		for (uint32_t i = 0; i < num_slots; ++i) {
			SDL_GPUVertexAttribute sdl_attribute;
			sdl_attribute.location = attribute.location + i;
			sdl_attribute.buffer_slot = slot;
			sdl_attribute.format = (attribute.type == ShaderDataType::Mat4)
									   ? SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4
									   : ShaderTypeUtils::get_sdl_format (
											 attribute.type
										 );

			sdl_attribute.offset = layout.stride;
			layout.sdl_attributes.push_back (sdl_attribute);

			layout.stride += bytes_per_slot;
		}
	}

	shaders[name] = shader;
}

Shader* ShaderManager::get_shader (const std::string& name) {
	Shader* shader = shaders.contains (name) ? &shaders[name] : nullptr;
	if (!shader)
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Shader not found: '%s'.", name.c_str ()
		);
	return shader;
}

void ShaderManager::add_shader (Shader& shader) {
	shaders.emplace (shader.name, shader);
}

SDL_GPUShader*
ShaderManager::compile_shader (const ShaderConfig& shader_config) const {
	SDL_IOStream* file = SDL_IOFromFile (shader_config.path.c_str (), "r");
	if (!file) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Failed to open shader: '%s'.",
			shader_config.path.c_str ()
		);
		SDL_CloseIO (file);
		return nullptr;
	}

	const Sint64 size = SDL_GetIOSize (file);
	if (size <= 0) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Shader file has invalid size: %s.",
			shader_config.path.c_str ()
		);
		SDL_CloseIO (file);
		return nullptr;
	}

	std::string source (size, '\0');
	const Sint64 read = SDL_ReadIO (file, source.data (), size);
	if (read != size) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Failed to read entire shader file: %s.",
			shader_config.path.c_str ()
		);
		SDL_CloseIO (file);
		return nullptr;
	}

	SDL_CloseIO (file);

	const SDL_GPUShaderCreateInfo info = {
		.code_size = static_cast<size_t> (size),
		.code = reinterpret_cast<const Uint8*> (source.data ()),
		.entrypoint = shader_config.entrypoint.c_str (),
		.format = shader_config.format,
		.stage = shader_config.stage,
		.num_samplers = shader_config.num_samplers,
		.num_storage_textures = shader_config.num_storage_textures,
		.num_storage_buffers = shader_config.num_storage_buffers,
		.num_uniform_buffers = shader_config.num_uniform_buffers,
		.props = shader_config.props
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader (device, &info);
	if (!shader)
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Shader compilation failed.");
	return shader;
}

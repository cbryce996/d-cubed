#include "shader.h"

#include "SDL3/SDL_log.h"

#include <fstream>
#include <nlohmann/json.hpp>

ShaderManager::ShaderManager (SDL_GPUDevice* device) : device (device) {}

ShaderManager::~ShaderManager () = default;

std::optional<DataTypes> ShaderManager::from_string (std::string_view string) {
	if (string == "float")
		return DataTypes::Float;
	if (string == "vec2")
		return DataTypes::Vec2;
	if (string == "vec3")
		return DataTypes::Vec3;
	if (string == "vec4")
		return DataTypes::Vec4;
	if (string == "mat4")
		return DataTypes::Mat4;
	if (string == "int")
		return DataTypes::Int;
	return std::nullopt;
}

uint32_t ShaderManager::size_bytes (DataTypes data_type) {
	switch (data_type) {
	case DataTypes::Float:
		return 4;
	case DataTypes::Vec2:
		return 8;
	case DataTypes::Vec3:
		return 12;
	case DataTypes::Vec4:
		return 16;
	case DataTypes::Mat4:
		return 64;
	case DataTypes::Int:
		return 4;
	default:
		return 0;
	}
}

void ShaderManager::load_sampled_textures (Shader& shader) {
	std::ifstream file (shader.json_path ());
	if (!file.is_open ())
		return;

	nlohmann::json data = nlohmann::json::parse (file);
	std::vector<SampledTexture> sampled_textures;

	if (data.contains ("textures")) {
		for (const auto& texture : data["textures"]) {
			SampledTexture sampled_texture;
			sampled_texture.name = texture["name"];
			sampled_texture.set = texture["set"];
			sampled_texture.binding = texture["binding"];
			sampled_textures.push_back (sampled_texture);
		}
	}

	shader.sampled_textures = sampled_textures;
}

void ShaderManager::load_vertex_buffers (Shader& shader) {
	std::ifstream file (shader.json_path ());
	if (!file.is_open ()) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR, "Failed to open shader metadata: '%s'.",
			shader.json_path ().c_str ()
		);
		return;
	}

	nlohmann::json data = nlohmann::json::parse (file);

	shader.vertex_buffers.clear ();

	if (!data.contains ("inputs")) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR, "Shader metadata '%s' missing 'inputs'.",
			shader.json_path ().c_str ()
		);
		return;
	}

	std::vector<VertexBufferField> vertex_buffer_fields;
	vertex_buffer_fields.reserve (data["inputs"].size ());

	for (const auto& input : data["inputs"]) {
		std::string json_type = input.value ("type", "");
		auto data_type = from_string (json_type);
		if (!data_type) {
			SDL_LogError (
				SDL_LOG_CATEGORY_ERROR,
				"Unsupported shader input type '%s' in %s.", json_type.c_str (),
				shader.json_path ().c_str ()
			);
			return;
		}

		VertexBufferField field;
		field.name = input.value ("name", "unknown");
		field.location = input.value ("location", 0u);
		field.type = *data_type;
		vertex_buffer_fields.push_back (std::move (field));
	}

	std::ranges::sort (vertex_buffer_fields, [] (auto& a, auto& b) {
		return a.location < b.location;
	});

	for (auto& field : vertex_buffer_fields) {
		const uint32_t slot = (field.location < 4) ? 0u : 1u;

		auto& [stride, input_rate, fields] = shader.vertex_buffers[slot];
		if (fields.empty ()) {
			input_rate = (slot == 0) ? InputRate::PerVertex
									 : InputRate::PerInstance;
			stride = 0;
		}

		field.offset = stride;
		stride += size_bytes (field.type);

		fields.push_back (field);
	}
}

void ShaderManager::load_uniform_buffers (Shader& shader) {
	std::ifstream file (shader.json_path ());
	if (!file.is_open ()) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR, "Failed to open shader metadata: '%s'.",
			shader.json_path ().c_str ()
		);
		return;
	}

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "Loading uniform blocks from: '%s'.",
		shader.json_path ().c_str ()
	);

	nlohmann::json data = nlohmann::json::parse (file);
	std::unordered_map<std::string, UniformBuffer> unform_buffers;

	if (data.contains ("ubos")) {
		for (const auto& ubo : data["ubos"]) {
			UniformBuffer uniform_buffer;
			uniform_buffer.name = ubo["name"];
			uniform_buffer.binding = ubo["binding"];
			uniform_buffer.total_size = ubo["block_size"];

			std::string type_id = ubo["type"];
			auto fields_json = data["types"][type_id]["members"];

			for (const auto& field_json : fields_json) {
				UniformBufferField field;
				field.name = field_json["name"];
				field.offset = field_json["offset"];

				if (std::string type = field_json["type"]; true) {
					auto maybe_type = from_string (type);
					if (!maybe_type) {
						SDL_LogError (
							SDL_LOG_CATEGORY_ERROR,
							"Unsupported uniform field type '%s' in %s.",
							type.c_str (), shader.json_path ().c_str ()
						);
						return;
					}

					field.type = *maybe_type;
					field.size = size_bytes (field.type);
				}

				uniform_buffer.fields[field.name] = field;
			}
			unform_buffers[uniform_buffer.name] = uniform_buffer;
		}
	}

	shader.uniform_buffers = unform_buffers;
}

bool ShaderManager::load_shader (
	Shader& vertex_shader, Shader& fragment_shader
) {
	if (get_shader (vertex_shader.name)) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"Vertex shader '%s' already loaded. Skipping.",
			vertex_shader.name.c_str ()
		);
		return false;
	}

	if (get_shader (fragment_shader.name)) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"Fragment shader '%s' already loaded. Skipping.",
			fragment_shader.name.c_str ()
		);
		return false;
	}

	load_sampled_textures (vertex_shader);
	load_sampled_textures (fragment_shader);
	if (vertex_shader.sampled_textures.empty ()) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"No sampled textures found for vertex shader: '%s'",
			vertex_shader.name.c_str ()
		);
	}
	if (fragment_shader.sampled_textures.empty ()) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"No sampled textures found for fragment shader: '%s'",
			fragment_shader.name.c_str ()
		);
	}

	load_uniform_buffers (vertex_shader);
	load_uniform_buffers (fragment_shader);
	if (vertex_shader.uniform_buffers.empty ()) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"No uniform buffers found for shader: '%s'",
			vertex_shader.name.c_str ()
		);
	}
	if (fragment_shader.uniform_buffers.empty ()) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER,
			"No uniform buffers found for shader: '%s'",
			fragment_shader.name.c_str ()
		);
	}

	load_vertex_buffers (vertex_shader);
	if (vertex_shader.vertex_buffers.empty ()) {
		SDL_LogWarn (
			SDL_LOG_CATEGORY_RENDER, "No vertex buffers found for shader: '%s'",
			vertex_shader.name.c_str ()
		);
	}

	compile_shader (vertex_shader);
	compile_shader (fragment_shader);

	shaders[vertex_shader.name] = vertex_shader;
	shaders[fragment_shader.name] = fragment_shader;
	return true;
}

Shader* ShaderManager::get_shader (const std::string& name) {
	Shader* shader = shaders.contains (name) ? &shaders[name] : nullptr;
	return shader;
}

void ShaderManager::add_shader (const Shader& shader) {
	shaders.emplace (shader.name, shader);
}

void ShaderManager::compile_shader (Shader& shader) const {
	SDL_LogMessage (
		SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO,
		"Compiling shader stage: %s", shader.path.c_str ()
	);

	SDL_IOStream* file = SDL_IOFromFile (shader.path.c_str (), "rb");
	if (!file) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Failed to open shader: '%s'.",
			shader.path.c_str ()
		);
		SDL_CloseIO (file);
		return;
	}

	const Sint64 size = SDL_GetIOSize (file);
	if (size <= 0) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Shader file has invalid size: %s.",
			shader.path.c_str ()
		);
		SDL_CloseIO (file);
		return;
	}

	std::string source (size, '\0');
	if (const Sint64 read = SDL_ReadIO (file, source.data (), size);
		read != size) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Failed to read entire shader file: %s.",
			shader.path.c_str ()
		);
		SDL_CloseIO (file);
		return;
	}

	SDL_CloseIO (file);

	const SDL_GPUShaderCreateInfo info = {
		.code_size = static_cast<size_t> (size),
		.code = reinterpret_cast<const Uint8*> (source.data ()),
		.entrypoint = shader.entrypoint.c_str (),
		.format = shader.format,
		.stage = shader.stage,
		.num_samplers = static_cast<Uint32> (shader.sampled_textures.size ()),
		.num_storage_textures = 0,
		.num_storage_buffers = 0,
		.num_uniform_buffers
		= static_cast<Uint32> (shader.uniform_buffers.size ()),
		.props = 0
	};

	shader.shader = SDL_CreateGPUShader (device, &info);

	if (!shader.shader) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Shader compilation failed for '%s': %s",
			shader.path.c_str (), SDL_GetError ()
		);
	} else {
		SDL_LogMessage (
			SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO,
			"Successfully compiled shader stage: %s", shader.path.c_str ()
		);
	}
}

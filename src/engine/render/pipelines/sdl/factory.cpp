#include <ranges>

#include "factory.h"

#include "render/shaders/shader.h"

SDLPipelineFactory::SDLPipelineFactory (
	SDL_GPUDevice* device, SDL_Window* window,
	const std::shared_ptr<ShaderManager>& shader_manager
)
	: device (device), window (window), shader_manager (shader_manager) {}

SDLPipelineFactory::~SDLPipelineFactory () = default;

SDL_GPUVertexInputRate
SDLPipelineFactory::to_sdl_input_rate (const InputRate input_rate) {
	switch (input_rate) {
	case InputRate::PerVertex:
		return SDL_GPU_VERTEXINPUTRATE_VERTEX;
	case InputRate::PerInstance:
		return SDL_GPU_VERTEXINPUTRATE_INSTANCE;
	}
	return SDL_GPU_VERTEXINPUTRATE_VERTEX;
}

SDL_GPUVertexElementFormat
SDLPipelineFactory::to_sdl_vertex_format (const DataTypes data_type) {
	switch (data_type) {
	case DataTypes::Float:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
	case DataTypes::Vec2:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	case DataTypes::Vec3:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	case DataTypes::Vec4:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	default:
		return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
	}
}

SDL_GPUTextureFormat
SDLPipelineFactory::to_sdl_texture_format (const TextureFormat format) {
	switch (format) {
	case TextureFormat::RGBA8:
		return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16F:
		return SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	case TextureFormat::BGRA8:
		return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	case TextureFormat::D32F:
		return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	case TextureFormat::Invalid:
	default:
		return SDL_GPU_TEXTUREFORMAT_INVALID;
	}
}

SDL_GPUCompareOp SDLPipelineFactory::to_sdl_compare_op (const CompareOp op) {
	switch (op) {
	case CompareOp::Never:
		return SDL_GPU_COMPAREOP_NEVER;
	case CompareOp::Less:
		return SDL_GPU_COMPAREOP_LESS;
	case CompareOp::Equal:
		return SDL_GPU_COMPAREOP_EQUAL;
	case CompareOp::LessOrEqual:
		return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	case CompareOp::Greater:
		return SDL_GPU_COMPAREOP_GREATER;
	case CompareOp::NotEqual:
		return SDL_GPU_COMPAREOP_NOT_EQUAL;
	case CompareOp::GreaterOrEqual:
		return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
	case CompareOp::Always:
		return SDL_GPU_COMPAREOP_ALWAYS;
	}
	return SDL_GPU_COMPAREOP_ALWAYS;
}

SDL_GPUCullMode SDLPipelineFactory::to_sdl_cull_mode (const CullMode mode) {
	switch (mode) {
	case CullMode::None:
		return SDL_GPU_CULLMODE_NONE;
	case CullMode::Front:
		return SDL_GPU_CULLMODE_FRONT;
	case CullMode::Back:
		return SDL_GPU_CULLMODE_BACK;
	}
	return SDL_GPU_CULLMODE_NONE;
}

SDL_GPUPrimitiveType
SDLPipelineFactory::to_sdl_primitive_type (const PrimitiveType type) {
	switch (type) {
	case PrimitiveType::TriangleList:
		return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	case PrimitiveType::TriangleStrip:
		return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
	case PrimitiveType::LineList:
		return SDL_GPU_PRIMITIVETYPE_LINELIST;
	case PrimitiveType::LineStrip:
		return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
	case PrimitiveType::PointList:
		return SDL_GPU_PRIMITIVETYPE_POINTLIST;
	}
	return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}

Pipeline*
SDLPipelineFactory::create_pipeline (const PipelineState& pipeline_state) {
	assert (
		shader_manager->get_shader (pipeline_state.material_state.vertex_shader)
	);
	assert (shader_manager->get_shader (
		pipeline_state.material_state.fragment_shader
	));

	const Shader* vertex_shader = shader_manager->get_shader (
		pipeline_state.material_state.vertex_shader
	);
	const Shader* fragment_shader = shader_manager->get_shader (
		pipeline_state.material_state.fragment_shader
	);

	if (!vertex_shader || !fragment_shader) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR,
			"Missing shader(s) for pipeline creation: vs='%s', fs='%s'",
			pipeline_state.material_state.vertex_shader.c_str (),
			pipeline_state.material_state.fragment_shader.c_str ()
		);
		return nullptr;
	}

	std::vector<SDL_GPUVertexBufferDescription> buffer_descriptions;
	buffer_descriptions.reserve (vertex_shader->vertex_buffers.size ());

	for (const auto& [slot, vertex_buffer] : vertex_shader->vertex_buffers) {
		SDL_GPUVertexBufferDescription description{};
		description.slot = slot;
		description.pitch = vertex_buffer.stride;
		description.input_rate = to_sdl_input_rate (vertex_buffer.input_rate);
		description.instance_step_rate = 0;

		buffer_descriptions.push_back (description);
	}

	std::vector<SDL_GPUVertexAttribute> sdl_attributes;
	for (const auto& [slot, vertex_buffer] : vertex_shader->vertex_buffers) {
		for (const auto& [name, location, type, offset] :
			 vertex_buffer.fields) {
			SDL_GPUVertexAttribute attribute{};
			attribute.location = location;
			attribute.buffer_slot = slot;
			attribute.format = to_sdl_vertex_format (type);
			attribute.offset = offset;

			if (attribute.format == SDL_GPU_VERTEXELEMENTFORMAT_INVALID) {
				SDL_LogError (
					SDL_LOG_CATEGORY_ERROR,
					"Invalid vertex format for attribute '%s' (loc %u) in "
					"shader '%s'",
					name.c_str (), location, vertex_shader->name.c_str ()
				);
				return nullptr;
			}

			sdl_attributes.push_back (attribute);
		}
	}

	SDL_GPUVertexInputState vertex_input_state{};
	vertex_input_state.vertex_buffer_descriptions = buffer_descriptions.data ();
	vertex_input_state.num_vertex_buffers = static_cast<Uint32> (
		buffer_descriptions.size ()
	);
	vertex_input_state.vertex_attributes = sdl_attributes.data ();
	vertex_input_state.num_vertex_attributes = static_cast<Uint32> (
		sdl_attributes.size ()
	);

	SDL_GPURasterizerState rasterizer_state{};
	rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	rasterizer_state.cull_mode = to_sdl_cull_mode (
		pipeline_state.material_state.cull_mode
	);
	rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	SDL_GPUDepthStencilState depth_stencil_state{};
	depth_stencil_state.compare_op = to_sdl_compare_op (
		pipeline_state.material_state.compare_op
	);
	depth_stencil_state.back_stencil_state = {};
	depth_stencil_state.front_stencil_state = {};
	depth_stencil_state.enable_depth_test
		= pipeline_state.material_state.enable_depth_test;
	depth_stencil_state.enable_depth_write
		= pipeline_state.material_state.enable_depth_write;
	depth_stencil_state.enable_stencil_test = false;

	std::vector<SDL_GPUColorTargetDescription> color_targets;
	color_targets.reserve (
		pipeline_state.render_pass_state.color_formats.size ()
	);

	for (const auto format : pipeline_state.render_pass_state.color_formats) {
		SDL_GPUColorTargetDescription desc{};
		desc.format = to_sdl_texture_format (format);
		desc.blend_state = {};
		color_targets.push_back (desc);
	}

	SDL_GPUGraphicsPipelineTargetInfo target_info{};
	target_info.num_color_targets = static_cast<Uint32> (color_targets.size ());
	target_info.color_target_descriptions = color_targets.data ();
	target_info.has_depth_stencil_target
		= pipeline_state.render_pass_state.has_depth_stencil_target;

	target_info.depth_stencil_format
		= pipeline_state.render_pass_state.has_depth_stencil_target
			  ? to_sdl_texture_format (
					pipeline_state.render_pass_state.depth_format
				)
			  : SDL_GPU_TEXTUREFORMAT_INVALID;

	SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.vertex_shader = vertex_shader->shader;
	pipeline_info.fragment_shader = fragment_shader->shader;
	pipeline_info.vertex_input_state = vertex_input_state;
	pipeline_info.primitive_type = to_sdl_primitive_type (
		pipeline_state.material_state.primitive_type
	);
	pipeline_info.rasterizer_state = rasterizer_state;
	pipeline_info.depth_stencil_state = depth_stencil_state;
	pipeline_info.multisample_state = {};
	pipeline_info.target_info = target_info;
	pipeline_info.props = 0;

	SDL_GPUGraphicsPipeline* sdl_pipeline = SDL_CreateGPUGraphicsPipeline (
		device, &pipeline_info
	);

	if (!sdl_pipeline) {
		SDL_LogError (
			SDL_LOG_CATEGORY_ERROR,
			"Failed to create graphics pipeline for material VS='%s' FS='%s'",
			pipeline_state.material_state.vertex_shader.c_str (),
			pipeline_state.material_state.fragment_shader.c_str ()
		);
		return nullptr;
	}

	return new Pipeline{sdl_pipeline};
}
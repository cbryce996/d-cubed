#include "pipeline.h"

#include "../mesh.h"

#include <ranges>

PipelineManager::PipelineManager (
	SDL_GPUDevice* device, SDL_Window* window,
	const std::shared_ptr<ShaderManager>& shader_manager
)
	: device (device), window (window), shader_manager (shader_manager.get ()) {
}

PipelineManager::~PipelineManager () = default;

Pipeline* PipelineManager::get_or_create (
	const RenderPassLayout& render_pass_layout, Material& material
) {
	if (Pipeline* pipeline = get_pipeline (render_pass_layout)) {
		return pipeline;
	}

	Pipeline* pipeline{
		.pipeline = create_pipeline (render_pass_layout, material)
	};

	add_pipeline (render_pass_layout, *pipeline);

	return pipeline;
}

Pipeline* PipelineManager::get_pipeline (const RenderPassLayout& render_pass_layout) {
	Pipeline* pipeline = pipelines.contains (render_pass_layout.key ()) ? &pipelines[render_pass_layout.key ()] : nullptr;
	if (!pipeline) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Pipeline not found.");
		return nullptr;
	}
	return pipeline;
}

void PipelineManager::add_pipeline (const RenderPassLayout& render_pass_layout, const Pipeline& pipeline) {
	pipelines.emplace (render_pass_layout.key (), pipeline);
}

SDL_GPUGraphicsPipeline*
PipelineManager::create_pipeline (const RenderPassLayout& render_pass_layout, Material& material) const {
	const Shader* shader = shader_manager->get_shader (material.shader_name);

	std::vector<SDL_GPUVertexBufferDescription> buffer_descriptions;
	for (auto const& [slot, layout] : shader->vertex_buffer_layouts) {
		SDL_GPUVertexBufferDescription description = {
			.slot = slot,
			.pitch = layout.stride,
			.input_rate = layout.input_rate,
			.instance_step_rate = 0
		};
		buffer_descriptions.push_back (description);
	}

	std::vector<SDL_GPUVertexAttribute> sdl_attributes;
	for (const auto& layout :
		 shader->vertex_buffer_layouts | std::views::values) {
		for (const auto& sdl_attr : layout.sdl_attributes) {
			sdl_attributes.push_back (sdl_attr);
		}
	}

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = buffer_descriptions.data (),
		.num_vertex_buffers = static_cast<Uint32> (buffer_descriptions.size ()),
		.vertex_attributes = sdl_attributes.data (),
		.num_vertex_attributes = static_cast<Uint32> (sdl_attributes.size ())
	};

	SDL_GPURasterizerState rasterizer_state = {
		.fill_mode = SDL_GPU_FILLMODE_FILL,
		.cull_mode = material.cull_mode,
		.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
	};

	SDL_GPUDepthStencilState depth_stencil_state = {
		.compare_op = material.compare_op,
		.back_stencil_state = {},
		.front_stencil_state = {},
		.enable_depth_test = material.enable_depth_test,
		.enable_depth_write = material.enable_depth_write,
		.enable_stencil_test = false
	};

	std::vector<SDL_GPUColorTargetDescription> color_targets;

	for (auto format : render_pass_layout.color_formats) {
		SDL_GPUColorTargetDescription desc{};
		desc.format = format;
		desc.blend_state = {};
		color_targets.push_back (desc);
	}

	SDL_GPUGraphicsPipelineTargetInfo target_info = {
		.num_color_targets = static_cast<Uint32> (color_targets.size ()),
		.color_target_descriptions = color_targets.data (),
		.depth_stencil_format = render_pass_layout.depth_format
									? SDL_GPU_TEXTUREFORMAT_D32_FLOAT
									: SDL_GPU_TEXTUREFORMAT_INVALID,
		.has_depth_stencil_target = render_pass_layout.has_depth_stencil_target
	};

	SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
		.vertex_shader = shader->vertex_shader,
		.fragment_shader = shader->fragment_shader,
		.vertex_input_state = vertex_input_state,
		.primitive_type = material.primitive_type,
		.rasterizer_state = rasterizer_state,
		.depth_stencil_state = depth_stencil_state,
		.multisample_state = {},
		.target_info = target_info,
		.props = 0
	};

	return SDL_CreateGPUGraphicsPipeline (device, &pipeline_info);
}
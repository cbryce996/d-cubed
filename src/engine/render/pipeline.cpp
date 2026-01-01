#include "pipeline.h"

#include "mesh.h"

PipelineManager::PipelineManager (
	SDL_GPUDevice* device, SDL_Window* window,
	const std::shared_ptr<ShaderManager>& shader_manager
)
	: device (device), window (window), shader_manager (shader_manager.get ()) {
}

PipelineManager::~PipelineManager () = default;

void PipelineManager::load_pipeline (
	const PipelineConfig* pipeline_config, const std::string& name
) {
	Pipeline pipeline{
		.pipeline = create_pipeline (*pipeline_config), .name = name
	};
	add_pipeline (pipeline);
}

Pipeline* PipelineManager::get_pipeline (const std::string& name) {
	Pipeline* pipeline = pipelines.contains (name) ? &pipelines[name] : nullptr;
	if (!pipeline) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Pipeline not found.");
		return nullptr;
	}
	return pipeline;
}

void PipelineManager::add_pipeline (Pipeline& pipeline) {
	pipelines.emplace (pipeline.name, pipeline);
}

SDL_GPUGraphicsPipeline*
PipelineManager::create_pipeline (const PipelineConfig& pipeline_config) const {
	const Shader* shader = pipeline_config.shader;

	// --- 1. Vertex Input Descriptions (How many buffers?) ---
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

	// --- 2. Vertex Attributes (What is inside the buffers?) ---
	std::vector<SDL_GPUVertexAttribute> sdl_attributes;
	for (auto const& [slot, layout] : shader->vertex_buffer_layouts) {
		for (const auto& sdl_attr : layout.sdl_attributes) {
			sdl_attributes.push_back (sdl_attr);
		}
	}

	// Combine them into the Vertex Input State
	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = buffer_descriptions.data (),
		.num_vertex_buffers = static_cast<Uint32> (buffer_descriptions.size ()),
		.vertex_attributes = sdl_attributes.data (),
		.num_vertex_attributes = static_cast<Uint32> (sdl_attributes.size ())
	};

	// --- 3. Graphics States (How should it be drawn?) ---
	SDL_GPURasterizerState rasterizer_state = {
		.fill_mode = SDL_GPU_FILLMODE_FILL,
		.cull_mode = pipeline_config.cull_mode,
		.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
	};

	SDL_GPUDepthStencilState depth_stencil_state = {
		.compare_op = pipeline_config.compare_op,
		.back_stencil_state = {},
		.front_stencil_state = {},
		.enable_depth_test = pipeline_config.enable_depth_test,
		.enable_depth_write = pipeline_config.enable_depth_write,
		.enable_stencil_test = false
	};

	// --- 4. Target Info (Where is it being drawn to?) ---
	SDL_GPUColorTargetDescription color_target_desc = {
		.format = SDL_GetGPUSwapchainTextureFormat (device, window),
		.blend_state = {}
	};

	SDL_GPUGraphicsPipelineTargetInfo target_info = {
		.num_color_targets = 1,
		.color_target_descriptions = &color_target_desc,
		.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.has_depth_stencil_target = true
	};

	// --- 5. Final Assembly ---
	SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
		.vertex_shader = shader->vertex_shader,
		.fragment_shader = shader->fragment_shader,
		.vertex_input_state = vertex_input_state,
		.primitive_type = pipeline_config.primitive_type,
		.rasterizer_state = rasterizer_state,
		.depth_stencil_state = depth_stencil_state,
		.multisample_state = {},
		.target_info = target_info,
		.props = 0
	};

	return SDL_CreateGPUGraphicsPipeline (device, &pipeline_info);
}
#include "pipeline.h"

#include "render.h"

PipelineManager::PipelineManager(
	SDL_GPUDevice* device,
	SDL_Window* window,
	const std::shared_ptr<ShaderManager>& shader_manager
)	: device(device),
	  window(window),
	  shader_manager(shader_manager.get()) {}

PipelineManager::~PipelineManager() = default;

Pipeline* PipelineManager::get_pipeline(const std::string& name) {
	Pipeline* pipeline = pipelines.contains(name) ? &pipelines[name] : nullptr;
	if (!pipeline) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Pipeline not found.");
		return nullptr;
	}
	return pipeline;
}

Pipeline* PipelineManager::get_or_create_pipeline(const Drawable* drawable) {
	Pipeline* pipeline = pipelines.contains(drawable->mesh->name) ? &pipelines[drawable->mesh->name] : nullptr;
	if (pipeline) {
		return pipeline;
	}

	pipeline = new Pipeline{};
	pipeline->pipeline = create_pipeline(drawable->material->pipeline_config);
	pipeline->name = "lit";

	add_pipeline(*pipeline);
	return pipeline;
}

void PipelineManager::add_pipeline(Pipeline& pipeline) {
	pipelines.emplace(pipeline.name, pipeline);
}

SDL_GPUGraphicsPipeline* PipelineManager::create_pipeline(PipelineConfig& config) const {
	SDL_GPUColorTargetDescription color_target_description{};
	color_target_description.format = SDL_GetGPUSwapchainTextureFormat(device, window);
	color_target_description.blend_state = {};

	SDL_GPUGraphicsPipelineTargetInfo pipeline_target_info{};
	pipeline_target_info.num_color_targets = 1;
	pipeline_target_info.color_target_descriptions = &color_target_description;
	pipeline_target_info.has_depth_stencil_target = true;
	pipeline_target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

	SDL_GPUVertexBufferDescription vertex_buffer_description{};
	vertex_buffer_description.slot = 0;
	vertex_buffer_description.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertex_buffer_description.pitch = sizeof(Vertex);

	SDL_GPUVertexAttribute vertex_attributes[3]{};
	vertex_attributes[0].location = 0;
	vertex_attributes[0].buffer_slot = 0;
	vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertex_attributes[0].offset = offsetof(Vertex, position);

	vertex_attributes[1].location = 1;
	vertex_attributes[1].buffer_slot = 0;
	vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertex_attributes[1].offset = offsetof(Vertex, normal);

	vertex_attributes[2].location = 2;
	vertex_attributes[2].buffer_slot = 0;
	vertex_attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertex_attributes[2].offset = offsetof(Vertex, color);

	SDL_GPUVertexInputState vertex_input_state{};
	vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_description;
	vertex_input_state.num_vertex_buffers = 1;
	vertex_input_state.vertex_attributes = vertex_attributes;
	vertex_input_state.num_vertex_attributes = 3;

	SDL_GPUGraphicsPipelineCreateInfo gp_info{};
	gp_info.vertex_input_state = vertex_input_state;
	gp_info.primitive_type = config.primitive_type;
	gp_info.vertex_shader = config.shader->vertex_shader;
	gp_info.fragment_shader = config.shader->fragment_shader;
	gp_info.rasterizer_state.cull_mode = config.cull_mode;
	gp_info.depth_stencil_state.enable_depth_test = config.enable_depth_test;
	gp_info.depth_stencil_state.enable_depth_write = config.enable_depth_write;
	gp_info.depth_stencil_state.compare_op = config.compare_op;
	gp_info.target_info.has_depth_stencil_target = true;
	gp_info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	gp_info.target_info = pipeline_target_info;
	gp_info.props = 0;

	return SDL_CreateGPUGraphicsPipeline(device, &gp_info);
}

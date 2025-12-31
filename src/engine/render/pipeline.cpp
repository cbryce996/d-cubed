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
	// Create vertex buffers
	SDL_GPUVertexBufferDescription vertex_buffers[2]{};
	;
	vertex_buffers[0] = {
		.slot = 0,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0,
		.pitch = sizeof (Block)
	};
	vertex_buffers[1] = {
		.slot = 1,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
		.instance_step_rate = 0,
		.pitch = sizeof (Block)
	};

	// Create vertex attributes
	uint32_t required_vertex_attributes = pipeline_config.shader->required_vertex_attributes;
	uint32_t required_instance_attributes = pipeline_config.shader->required_instance_attributes;
	uint32_t block_size = pipeline_config.shader->block_size;

	uint32_t total_attributes = required_vertex_attributes + required_instance_attributes;

	std::vector<SDL_GPUVertexAttribute> vertex_attributes(total_attributes);

	for (uint32_t i = 0; i < required_vertex_attributes; i++) {
		vertex_attributes[i] = {
			.location = i,
			.buffer_slot = 0,
			.format = get_format (),
			.offset = i * block_size
		};
	}
	for (uint32_t i = 0; i < required_instance_attributes; i++) {
		vertex_attributes[i + required_vertex_attributes] = {
			.location = i + required_vertex_attributes,
			.buffer_slot = 1,
			.format = get_format (),
			.offset = i * block_size
		};
	}

	SDL_GPUColorTargetDescription color_target_description = {
		.format = SDL_GetGPUSwapchainTextureFormat (device, window),
		.blend_state = {}
	};

	SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_info = {
		.vertex_shader = pipeline_config.shader->vertex_shader,
		.fragment_shader = pipeline_config.shader->fragment_shader,
		.vertex_input_state
		= {.vertex_buffer_descriptions = vertex_buffers,
		   .num_vertex_buffers = 2,
		   .vertex_attributes = vertex_attributes.data(),
		   .num_vertex_attributes = total_attributes},
		.primitive_type = pipeline_config.primitive_type,
		.rasterizer_state
		= {.cull_mode = pipeline_config.cull_mode,
		   .fill_mode = SDL_GPU_FILLMODE_FILL},
		.depth_stencil_state
		= {.compare_op = pipeline_config.compare_op,
		   .enable_depth_test = pipeline_config.enable_depth_test,
		   .enable_depth_write = pipeline_config.enable_depth_write},
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = &color_target_description,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.has_depth_stencil_target = true
		}
	};

	return SDL_CreateGPUGraphicsPipeline (device, &graphics_pipeline_info);
}
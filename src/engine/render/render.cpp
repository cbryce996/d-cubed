#include "render.h"

#include "../memory.h"
#include "material.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cameras/camera.h"
#include "utils.h"

RenderManager::RenderManager (
	SDL_GPUDevice* device, SDL_Window* window,
	std::shared_ptr<ShaderManager> shader_manager,
	std::shared_ptr<PipelineManager> pipeline_manager,
	std::shared_ptr<BufferManager> buffer_manager,
	std::shared_ptr<CameraManager> camera_manager,
	std::shared_ptr<AssetManager> asset_manager
)
	: shader_manager (std::move (shader_manager)),
	  pipeline_manager (std::move (pipeline_manager)),
	  buffer_manager (std::move (buffer_manager)),
	  camera_manager (std::move (camera_manager)),
	  asset_manager (std::move (asset_manager)), device (device),
	  window (window) {
	SDL_GetWindowSize (window, &width, &height);

	load_shaders ();
	load_pipelines ();
	create_depth_texture ();
	setup_render_graph ();
}

RenderManager::~RenderManager () = default;

void RenderManager::load_pipelines () const {
	pipeline_manager->load_pipeline (
		new PipelineConfig{
			.shader = shader_manager->get_shader ("anomaly"),
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.cull_mode = SDL_GPU_CULLMODE_BACK,
			.depth_compare = SDL_GPU_COMPAREOP_LESS,
			.depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.enable_depth_test = true,
			.enable_depth_write = true,
			.has_depth_stencil_target = true,
		},
		"lit_opaque_backcull"
	);
}

void RenderManager::load_shaders () const {
	const std::string shader_base = SHADERS_DIR;

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "Loading shaders.", shader_base.c_str ()
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/cube.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 2
		},
		ShaderConfig{
			.path = shader_base + "bin/cube.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 2
		},
		"anomaly"
	);
}

void RenderManager::setup_render_graph () {
	RenderPassNode render_pass;
	render_pass.name = "geometry_pass";
	render_pass.execute = [this] (const RenderContext& render_context) {
		const Camera* active_camera
			= render_context.camera_manager->get_active_camera ();
		const float aspect_ratio = static_cast<float> (width)
								   / static_cast<float> (height);
		const glm::vec3 light_pos_world = active_camera->transform.position;

		current_render_pass = create_render_pass ();
		set_viewport (current_render_pass);

		// Create view uniform
		glm::mat4 view_projection = CameraManager::compute_view_projection (
			*active_camera, aspect_ratio
		);
		Collection view_uniform_builder{};
		view_uniform_builder.push (view_projection);

		// Create global uniform
		Collection global_uniform_builder{};
		global_uniform_builder.push (glm::vec4 (light_pos_world, 1.0f));
		global_uniform_builder.push (
			render_context.time
		);
		global_uniform_builder.push(
			glm::vec4(render_context.camera_manager->get_active_camera ()->transform.position, 0.0f)
		);

		std::vector<UniformBinding> uniform_bindings;

		UniformBinding view_uniform_binding{};
		view_uniform_binding.data = &view_uniform_builder.storage;
		view_uniform_binding.slot = 0;
		view_uniform_binding.size = sizeof (Block);
		view_uniform_binding.stage = ShaderStage::Both;
		uniform_bindings.push_back (view_uniform_binding);

		UniformBinding global_uniform_binding{};
		global_uniform_binding.data = &global_uniform_builder.storage;
		view_uniform_binding.slot = 1;
		global_uniform_binding.size = sizeof (Block);
		global_uniform_binding.stage = ShaderStage::Both;
		uniform_bindings.push_back (global_uniform_binding);

		for (const Drawable& drawable : *render_context.drawables) {
			const Pipeline* pipeline
				= render_context.pipeline_manager->get_pipeline (
					drawable.material->pipeline_name
				);
			const Buffer* vertex_buffer = drawable.vertex_buffer;
			const Buffer* instance_buffer = drawable.instance_buffer;

			draw (
				pipeline, vertex_buffer, instance_buffer, &drawable,
				&uniform_bindings
			);
		}

		SDL_EndGPURenderPass (current_render_pass);
		SDL_SubmitGPUCommandBuffer (
			render_context.buffer_manager->command_buffer
		);
	};
	render_graph.add_pass (render_pass);
}

void RenderManager::resize (int new_width, int new_height) {
	width = new_width;
	height = new_height;

	SDL_WaitForGPUIdle (device);

	if (buffer_manager->depth_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->depth_texture);
		buffer_manager->depth_texture = nullptr;
	}

	create_depth_texture ();
}

void RenderManager::acquire_swap_chain () {
	SDL_GPUTexture* swap_chain_texture = nullptr;
	Uint32 w = static_cast<Uint32> (width);
	Uint32 h = static_cast<Uint32> (height);

	if (!SDL_WaitAndAcquireGPUSwapchainTexture (
			buffer_manager->command_buffer, window, &swap_chain_texture, &w, &h
		)) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Failed to acquire swap chain texture."
		);
		return;
	}

	width = static_cast<int> (w);
	height = static_cast<int> (h);
	buffer_manager->swap_chain_texture = swap_chain_texture;
}

void RenderManager::create_depth_texture () const {
	if (buffer_manager->depth_texture)
		return;

	SDL_GPUTextureCreateInfo depth_info{};
	depth_info.type = SDL_GPU_TEXTURETYPE_2D;
	depth_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depth_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	depth_info.width = width;
	depth_info.height = height;
	depth_info.layer_count_or_depth = 1;
	depth_info.num_levels = 1;
	depth_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

	buffer_manager->depth_texture = SDL_CreateGPUTexture (device, &depth_info);
}

SDL_GPURenderPass* RenderManager::create_render_pass () const {
	if (!buffer_manager->depth_texture) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Depth texture not created.");
		return nullptr;
	}

	if (!buffer_manager->swap_chain_texture) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Swap chain texture not created."
		);
		return nullptr;
	}

	SDL_GPUDepthStencilTargetInfo depth_target_info{};
	depth_target_info.texture = buffer_manager->depth_texture;
	depth_target_info.clear_depth = 1.0f;
	depth_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
	depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target_info.cycle = true;

	SDL_GPUColorTargetInfo color_target_info{};
	color_target_info.texture = buffer_manager->swap_chain_texture;
	color_target_info.mip_level = 0;
	color_target_info.layer_or_depth_plane = 0;
	color_target_info.clear_color = {0.02f, 0.02f, 0.05f, 1.0f};
	color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
	color_target_info.store_op = SDL_GPU_STOREOP_STORE;
	color_target_info.resolve_texture = nullptr;
	color_target_info.resolve_mip_level = 0;
	color_target_info.resolve_layer = 0;
	color_target_info.cycle = true;
	color_target_info.cycle_resolve_texture = true;

	return SDL_BeginGPURenderPass (
		buffer_manager->command_buffer, &color_target_info, 1,
		&depth_target_info
	);
}

void RenderManager::draw (
	const Pipeline* pipeline, const Buffer* vertex_buffer,
	const Buffer* instance_buffer, const Drawable* drawable,
	const std::vector<UniformBinding>* uniform_bindings
) {
	if (!pipeline || !vertex_buffer || !instance_buffer) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Missing pipeline or buffers.");
		return;
	}

	// Create render pass and bind to pipeline
	SDL_GPURenderPass* pass = current_render_pass;
	SDL_BindGPUGraphicsPipeline (pass, pipeline->pipeline);

	// Push uniform bindings
	for (const auto & [slot, data, size, stage] : *uniform_bindings) {
			if (stage == ShaderStage::Vertex
			|| stage == ShaderStage::Both) {
			SDL_PushGPUVertexUniformData (
				buffer_manager->command_buffer, slot,
				data, size
			);
		}

		if (stage == ShaderStage::Fragment
			|| stage == ShaderStage::Both) {
			SDL_PushGPUFragmentUniformData (
				buffer_manager->command_buffer, slot,
				data, size
			);
		}
	}

	// Vertex buffer with instance
	SDL_GPUBufferBinding bindings[2] = {
		{vertex_buffer->gpu_buffer.buffer, 0},
		{instance_buffer->gpu_buffer.buffer, 0}
	};

	SDL_BindGPUVertexBuffers (pass, 0, bindings, 2);

	SDL_DrawGPUPrimitives (
		pass, drawable->mesh->vertex_count,
		static_cast<Uint32> (drawable->instances_count), 0, 0
	);
}

void RenderManager::prepare_drawables (std::vector<Drawable>& drawables) const {
	for (Drawable& drawable : drawables) {
		// Create and write instance buffer
		Buffer* instance_buffer
			= buffer_manager->get_or_create_instance_buffer (&drawable);
		drawable.instance_buffer = instance_buffer;

		buffer_manager->write (
			drawable.instances.data (), drawable.instances_size, instance_buffer
		);

		buffer_manager->upload (instance_buffer);

		// Create and write vertex buffer
		Buffer* vertex_buffer = buffer_manager->get_or_create_vertex_buffer (
			&drawable
		);
		drawable.vertex_buffer = vertex_buffer;

		buffer_manager->write (
			drawable.mesh->vertex_data, drawable.mesh->vertex_size,
			vertex_buffer
		);

		buffer_manager->upload (vertex_buffer);
	}
}

void RenderManager::set_viewport (SDL_GPURenderPass* current_render_pass) {
	SDL_GPUViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.w = static_cast<float> (width);
	viewport.h = static_cast<float> (height);
	viewport.min_depth = 0.0f;
	viewport.max_depth = 1.0f;
	SDL_SetGPUViewport (current_render_pass, &viewport);
}

void RenderManager::render (RenderState* render_state, float delta_time) {
	buffer_manager->command_buffer = SDL_AcquireGPUCommandBuffer (device);
	acquire_swap_chain ();

	prepare_drawables (render_state->drawables);

	RenderContext render_context{
		.camera_manager = camera_manager.get (),
		.pipeline_manager = pipeline_manager.get (),
		.buffer_manager = buffer_manager.get (),
		.shader_manager = shader_manager.get (),
		.drawables = &render_state->drawables,
		.width = width,
		.height = height,
		.time = delta_time
	};
	render_graph.execute_all (render_context);
}

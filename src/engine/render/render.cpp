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
	assert(device && "GPU device must not be null");
	assert(window && "Window must not be null");

	assert(this->shader_manager && "Expected shader manager to be created");
	assert(this->pipeline_manager && "Expected pipeline manager to be created");
	assert(this->buffer_manager && "bExpected buffer manager to be created");
	assert(this->camera_manager && "Expected camera manager to be created");
	assert(this->asset_manager && "Expected asset manager to be created");

	SDL_GetWindowSize(window, &width, &height);
	assert(width > 0 && height > 0 && "Expected width and height to be a positive integer");

	load_shaders ();
	load_pipelines ();
	create_depth_texture ();
	create_gbuffer_textures (width, height);
	setup_render_graph ();
}

RenderManager::~RenderManager () = default;

void RenderManager::load_pipelines () const {
	pipeline_manager->load_pipeline (
		new PipelineConfig{
			.shader = shader_manager->get_shader ("cube_geometry"),
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.cull_mode = SDL_GPU_CULLMODE_BACK,
			.depth_compare = SDL_GPU_COMPAREOP_LESS,
			.depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.color_formats = {
				SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
				SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
				SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM
			},
			.enable_depth_test = true,
			.enable_depth_write = true,
			.has_depth_stencil_target = true,
		},
		"cube_geometry"
	);
	pipeline_manager->load_pipeline (
		new PipelineConfig{
			.shader = shader_manager->get_shader ("cube_lighting"),
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.cull_mode = SDL_GPU_CULLMODE_BACK,
			.depth_compare = SDL_GPU_COMPAREOP_LESS,
			.depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.color_formats = {SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM},
			.enable_depth_test = false,
			.enable_depth_write = false,
			.has_depth_stencil_target = false,
		},
		"cube_lighting"
	);

	assert(pipeline_manager->get_pipeline("cube_geometry") && "Expected cube geometry pipeline to be available");
	assert(pipeline_manager->get_pipeline("cube_lighting") && "Expected cube lighting pipeline to be available");
}

void RenderManager::load_shaders () const {
	const std::string shader_base = SHADERS_DIR;

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "Loading shaders.", shader_base.c_str ()
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/test_geometry.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 2
		},
		ShaderConfig{
			.path = shader_base + "bin/test_geometry.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 2
		},
		"cube_geometry"
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/test_lighting.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 2
		},
		ShaderConfig{
			.path = shader_base + "bin/test_lighting.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 2,
			.num_samplers = 3
		},
		"cube_lighting"
	);

	assert(shader_manager->get_shader("cube_geometry") && "Expected cube geometry shader to be available");
	assert(shader_manager->get_shader("cube_lighting") && "Expected cube lighting shader to be available");
}

void RenderManager::create_gbuffer_textures (int width, int height) {
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.width = width;
	info.height = height;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.usage = info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	info.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	buffer_manager->g_position_texture = SDL_CreateGPUTexture (device, &info);
	assert(buffer_manager->g_position_texture && "Failed to create position texture");

	info.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	buffer_manager->g_normal_texture = SDL_CreateGPUTexture (device, &info);
	assert(buffer_manager->g_position_texture && "Failed to create normal texture");

	info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	buffer_manager->g_albedo_texture = SDL_CreateGPUTexture (device, &info);
	assert(buffer_manager->g_position_texture && "Failed to create albedo texture");

	if (!buffer_manager->linear_sampler) {
		SDL_GPUSamplerCreateInfo sampler_info{};
		sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
		sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
		sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
		sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
		sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
		sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

		buffer_manager->linear_sampler = SDL_CreateGPUSampler (
			device, &sampler_info
		);
		assert(buffer_manager->g_position_texture && "Expected linear sampler to have been created");
	}

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "G-buffer textures created (%dx%d)", width,
		height
	);
}

void RenderManager::destroy_gbuffer_textures () {
	if (buffer_manager->g_position_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->g_position_texture);
		buffer_manager->g_position_texture = nullptr;
	}

	if (buffer_manager->g_normal_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->g_normal_texture);
		buffer_manager->g_normal_texture = nullptr;
	}

	if (buffer_manager->g_albedo_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->g_albedo_texture);
		buffer_manager->g_albedo_texture = nullptr;
	}
}

void RenderManager::setup_render_graph () {
	RenderPassNode setup_uniforms_pass;
	setup_uniforms_pass.name = "setup_uniforms_pass";
	setup_uniforms_pass.type = RenderPassType::Setup;
	setup_uniforms_pass.execute = [this] (const RenderContext& render_context) {
		assert(render_context.camera_manager->get_active_camera () && "Expected an active camera to be available");

		const Camera* active_camera
				= render_context.camera_manager->get_active_camera ();
		const float aspect_ratio = static_cast<float> (width)
								   / static_cast<float> (height);
		const glm::vec3 light_pos_world = active_camera->transform.position;

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
			glm::vec4(active_camera->transform.position, 0.0f)
		);

		std::vector<UniformBinding> uniform_bindings;

		UniformBinding view_uniform_binding{};
		view_uniform_binding.data = &view_uniform_builder.storage;
		view_uniform_binding.slot = 1;
		view_uniform_binding.size = sizeof (Block);
		view_uniform_binding.stage = ShaderStage::Vertex;
		uniform_bindings.push_back (view_uniform_binding);

		UniformBinding global_uniform_binding{};
		global_uniform_binding.data = &global_uniform_builder.storage;
		global_uniform_binding.slot = 0;
		global_uniform_binding.size = sizeof (Block);
		global_uniform_binding.stage = ShaderStage::Both;
		uniform_bindings.push_back (global_uniform_binding);

		push_uniform_bindings(uniform_bindings);
	};

	RenderPassNode geometry_pass;
	geometry_pass.name = "geometry_pass";
	geometry_pass.type = RenderPassType::Geometry;
	geometry_pass.dependencies = {"setup_uniforms_pass"};
	geometry_pass.execute =
		[this] (const RenderContext& render_context) {
			assert(buffer_manager->g_position_texture && "Expected position texture to have been created");
			assert(buffer_manager->g_normal_texture && "Expected normal texture to have been created");
			assert(buffer_manager->g_albedo_texture && "Expected albedo texture to have been created");
			assert(buffer_manager->depth_texture && "Expected depth texture to have been created");

			// Create pass config
			RenderPassConfig geometry_pass_config{};
			geometry_pass_config.color_targets = {
				buffer_manager->g_position_texture,
				buffer_manager->g_normal_texture,
				buffer_manager->g_albedo_texture
			};
			geometry_pass_config.depth_target = buffer_manager->depth_texture;
			geometry_pass_config.clear_color = {0.0f, 0.0f, 0.0f, 0.0f};
			geometry_pass_config.clear_depth = true;

			// Create render pass
			current_render_pass = create_render_pass (geometry_pass_config);
			set_viewport (current_render_pass);

			// Draw drawables
			for (const Drawable& drawable : *render_context.drawables) {
				assert(render_context.pipeline_manager->get_pipeline ("cube_geometry") && "Expected pipeline to available");
				const Pipeline* pipeline = render_context.pipeline_manager
											   ->get_pipeline ("cube_geometry");
				const Buffer* vertex_buffer = drawable.vertex_buffer;
				const Buffer* instance_buffer = drawable.instance_buffer;

				draw_mesh (
					pipeline, vertex_buffer, instance_buffer, &drawable
				);
			}

			SDL_EndGPURenderPass (current_render_pass);
		};

	RenderPassNode lighting_pass;
	lighting_pass.name = "lighting_pass";
	lighting_pass.type = RenderPassType::Lighting;
	lighting_pass.dependencies = {"geometry_pass"};
	lighting_pass.execute = [this] (const RenderContext& render_context) {
		assert(buffer_manager->swap_chain_texture && "Expected swap chain texture to have been created");

		// Create pass config
		RenderPassConfig lighting_pass_config{};
		lighting_pass_config.color_targets = {
			buffer_manager->swap_chain_texture
		};
		lighting_pass_config.depth_target = nullptr;
		lighting_pass_config.clear_color = {0.0f, 0.0f, 0.0f, 0.0f};
		lighting_pass_config.clear_depth = false;

		// Create viewport
		current_render_pass = create_render_pass (lighting_pass_config);
		set_viewport (current_render_pass);

		assert(render_context.pipeline_manager->get_pipeline ("cube_lighting") && "Expected pipeline to available");
		const Pipeline* pipeline = render_context.pipeline_manager
											   ->get_pipeline ("cube_lighting");

		// Draw screen
		draw_screen (
			pipeline
		);

		SDL_EndGPURenderPass (current_render_pass);
	};

	render_graph.add_pass (setup_uniforms_pass);
	assert(render_graph.get_render_pass("setup_uniforms_pass") && "Expected render pass to be available");

	render_graph.add_pass (geometry_pass);
	assert(render_graph.get_render_pass("geometry_pass") && "Expected render pass to be available");

	render_graph.add_pass (lighting_pass);
	assert(render_graph.get_render_pass("lighting_pass") && "Expected render pass to be available");
}

void RenderManager::resize (int new_width, int new_height) {
	width = new_width;
	height = new_height;

	SDL_WaitForGPUIdle (device);

	if (buffer_manager->depth_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->depth_texture);
		buffer_manager->depth_texture = nullptr;
		assert(buffer_manager && "Expected depth texture to be released");
	}

	destroy_gbuffer_textures ();
	create_gbuffer_textures (width, height);
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
			SDL_LOG_CATEGORY_RENDER, "Failed to acquire swap chain texture (window may be minimized)"
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
	assert(buffer_manager->depth_texture && "Expected depth texture to be craeted");

}

SDL_GPURenderPass* RenderManager::create_render_pass (
	const RenderPassConfig& render_pass_config
) const {
	assert(buffer_manager->command_buffer && "Expected command buffer to be acquired");
	assert(buffer_manager->depth_texture && "Expected depth texture to be created");
	assert(buffer_manager->swap_chain_texture && "Expected swap chain texture to be created");

	assert(!render_pass_config.color_targets.empty() && "Expected color targets not to be empty");

	std::vector<SDL_GPUColorTargetInfo> color_target_infos;
	color_target_infos.reserve (render_pass_config.color_targets.size ());

	for (SDL_GPUTexture* texture : render_pass_config.color_targets) {
		SDL_GPUColorTargetInfo info{};
		info.texture = texture;
		info.mip_level = 0;
		info.layer_or_depth_plane = 0;
		info.clear_color = render_pass_config.clear_color;
		info.load_op = SDL_GPU_LOADOP_CLEAR;
		info.store_op = SDL_GPU_STOREOP_STORE;
		info.resolve_texture = nullptr;
		info.cycle = true;
		info.cycle_resolve_texture = false;
		color_target_infos.push_back (info);
	}

	if (render_pass_config.depth_target) {
		SDL_GPUDepthStencilTargetInfo depth_target_info{};
		depth_target_info.texture = render_pass_config.depth_target;
		depth_target_info.clear_depth = render_pass_config.clear_depth ? 1.0f
																	   : 0.0f;
		depth_target_info.load_op = render_pass_config.clear_depth
										? SDL_GPU_LOADOP_CLEAR
										: SDL_GPU_LOADOP_LOAD;
		depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
		depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
		depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
		depth_target_info.cycle = true;

		return SDL_BeginGPURenderPass (
			buffer_manager->command_buffer, color_target_infos.data (),
			static_cast<uint32_t> (color_target_infos.size ()),
			&depth_target_info
		);
	}

	return SDL_BeginGPURenderPass (
		buffer_manager->command_buffer, color_target_infos.data (),
		static_cast<uint32_t> (color_target_infos.size ()), nullptr
	);
}

void RenderManager::push_uniform_bindings(
	std::vector<UniformBinding>& uniform_bindings
) const {
	assert(!uniform_bindings.empty() && "Expected uniform bindings to not be empty");

	for (const auto& [name, slot, data, size, stage] : uniform_bindings) {
		assert(data && "Expected uniform binding to have data");
		assert(size % UNIFORM_ALIGNMENT == 0 && "Expected uniform binding data to be a multiple of 16 bytes");
		assert(size > 0 && "Expected uniform binding to have a size");

		if (stage == ShaderStage::Vertex || stage == ShaderStage::Both) {
			SDL_PushGPUVertexUniformData (
				buffer_manager->command_buffer, slot, data, size
			);
		}

		if (stage == ShaderStage::Fragment || stage == ShaderStage::Both) {
			SDL_PushGPUFragmentUniformData (
				buffer_manager->command_buffer, slot, data, size
			);
		}
	}
}

void RenderManager::draw_mesh (
	const Pipeline* pipeline, const Buffer* vertex_buffer,
	const Buffer* instance_buffer, const Drawable* drawable
) {
	assert(pipeline && "Expected pipeline to be created");
	assert(vertex_buffer && "Expected vertex buffer to be created");
	assert(instance_buffer && "Expected instance buffer to be created");
	assert(drawable && "Expected drawables to be created");

	// Create render pass and bind to pipeline
	SDL_BindGPUGraphicsPipeline (current_render_pass, pipeline->pipeline);

	// Bing vertex buffer with instance
	SDL_GPUBufferBinding bindings[2] = {
		{vertex_buffer->gpu_buffer.buffer, 0},
		{instance_buffer->gpu_buffer.buffer, 0}
	};

	SDL_BindGPUVertexBuffers (current_render_pass, 0, bindings, 2);

	SDL_DrawGPUPrimitives (
		current_render_pass, drawable->mesh->vertex_count,
		static_cast<Uint32> (drawable->instances_count), 0, 0
	);
}

void RenderManager::draw_screen(
	const Pipeline* pipeline
) {
	assert(pipeline && "Expected pipeline to be created");

	// Create render pass and bind to pipeline
	SDL_BindGPUGraphicsPipeline (current_render_pass, pipeline->pipeline);

	// Bind GBuffer texture samplers
	SDL_GPUTextureSamplerBinding samplers[3];
	samplers[0].texture = buffer_manager->g_position_texture;
	samplers[0].sampler = buffer_manager->linear_sampler;
	samplers[1].texture = buffer_manager->g_normal_texture;
	samplers[1].sampler = buffer_manager->linear_sampler;
	samplers[2].texture = buffer_manager->g_albedo_texture;
	samplers[2].sampler = buffer_manager->linear_sampler;
	SDL_BindGPUFragmentSamplers (current_render_pass, 0, samplers, 3);

	SDL_DrawGPUPrimitives (
		current_render_pass, 3,
		1, 0, 0
	);
}

void RenderManager::prepare_drawables (std::vector<Drawable>& drawables) const {
	assert(!drawables.empty () && "Expected drawables to not be empty");

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
	assert (current_render_pass && "Expceted current render pass to be crated");

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
	assert(render_state && "Expected render state to have been created");

	buffer_manager->command_buffer = SDL_AcquireGPUCommandBuffer (device);
	assert(buffer_manager->command_buffer && "Expected command buffer to have been acquired");

	acquire_swap_chain ();
	assert(buffer_manager->swap_chain_texture && "Expected swap chain texture to have been acquired");

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

	SDL_SubmitGPUCommandBuffer(buffer_manager->command_buffer);

	buffer_manager->command_buffer = nullptr;
}

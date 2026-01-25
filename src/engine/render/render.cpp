#include "render.h"

#include "block.h"
#include "buffers/buffer.h"
#include "context.h"
#include "drawable.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>

#include "cameras/camera.h"
#include "pipelines/pipeline.h"
#include "shaders/shader.h"

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
	assert (device);
	assert (window);

	assert (this->shader_manager);
	assert (this->pipeline_manager);
	assert (this->buffer_manager);
	assert (this->camera_manager);
	assert (this->asset_manager);

	SDL_GetWindowSize (window, &width, &height);
	assert (width > 0 && height > 0);

	load_shaders ();
	create_depth_texture ();
	create_gbuffer_textures (width, height);
	setup_render_graph ();
}

RenderManager::~RenderManager () = default;

void RenderManager::load_shaders () const {
	const std::string shader_base = SHADERS_DIR;

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "Loading shaders.", shader_base.c_str ()
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/geometry.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 2
		},
		ShaderConfig{
			.path = shader_base + "bin/gbuffer.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 2
		},
		"geometry"
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/screen.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 2
		},
		ShaderConfig{
			.path = shader_base + "bin/lighting.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 2,
			.num_samplers = 3
		},
		"lighting"
	);

	assert (shader_manager->get_shader ("geometry"));
	assert (shader_manager->get_shader ("lighting"));
}

void RenderManager::create_gbuffer_textures (
	const int width, const int height
) const {
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.width = width;
	info.height = height;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.usage = info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
							  | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	info.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	buffer_manager->g_position_texture = SDL_CreateGPUTexture (device, &info);
	assert (buffer_manager->g_position_texture);

	info.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	buffer_manager->g_normal_texture = SDL_CreateGPUTexture (device, &info);
	assert (buffer_manager->g_normal_texture);

	info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	buffer_manager->g_albedo_texture = SDL_CreateGPUTexture (device, &info);
	assert (buffer_manager->g_albedo_texture);

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
		assert (buffer_manager->linear_sampler);
	}

	SDL_LogInfo (
		SDL_LOG_CATEGORY_RENDER, "G-buffer textures created (%dx%d)", width,
		height
	);
}

void RenderManager::destroy_gbuffer_textures () const {
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
	render_graph.add_pass (RenderPasses::UniformPass);
	assert (render_graph.get_render_pass ("uniform_pass"));
	assert (RenderPasses::UniformPass.execute);

	render_graph.add_pass (RenderPasses::GeometryPass);
	assert (render_graph.get_render_pass ("geometry_pass"));
	assert (RenderPasses::GeometryPass.execute);

	render_graph.add_pass (RenderPasses::DeferredPass);
	assert (render_graph.get_render_pass ("deferred_pass"));
	assert (RenderPasses::DeferredPass.execute);
}

void RenderManager::resize (const int new_width, const int new_height) {
	width = new_width;
	height = new_height;

	SDL_WaitForGPUIdle (device);

	if (buffer_manager->depth_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->depth_texture);
		buffer_manager->depth_texture = nullptr;
		assert (buffer_manager);
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
			SDL_LOG_CATEGORY_RENDER,
			"Failed to acquire swap chain texture (window may be minimized)"
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
	assert (buffer_manager->depth_texture);
}

void RenderManager::prepare_drawables (std::vector<Drawable>& drawables) const {
	for (Drawable& drawable : drawables) {
		assert (drawable.mesh);
		assert (&drawable.instance_blocks);

		// --- Instance buffer ---
		drawable.instance_buffer
			= buffer_manager->get_or_create_instance_buffer (drawable);

		buffer_manager->write (
			drawable.instance_blocks.data (),
			drawable.instance_blocks.size () * sizeof (Block),
			*drawable.instance_buffer
		);
		buffer_manager->upload (*drawable.instance_buffer);

		// --- Vertex buffer ---
		drawable.vertex_buffer = buffer_manager->get_or_create_vertex_buffer (
			*drawable.mesh
		);

		buffer_manager->write (
			drawable.mesh->gpu_state.vertices.data (),
			drawable.mesh->gpu_state.vertices.size () * sizeof (Block),
			*drawable.vertex_buffer
		);
		buffer_manager->upload (*drawable.vertex_buffer);

		// --- Index buffer ---
		drawable.index_buffer = buffer_manager->get_or_create_index_buffer (
			*drawable.mesh
		);

		buffer_manager->write (
			drawable.mesh->gpu_state.indices.data (),
			drawable.mesh->gpu_state.indices.size () * sizeof (uint32_t),
			*drawable.index_buffer
		);
		buffer_manager->upload (*drawable.index_buffer);
	}
}

void RenderManager::render (RenderState* render_state, float delta_time) {
	assert (render_state);

	buffer_manager->command_buffer = SDL_AcquireGPUCommandBuffer (device);
	assert (buffer_manager->command_buffer);

	acquire_swap_chain ();
	assert (buffer_manager->swap_chain_texture);

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

	SDL_SubmitGPUCommandBuffer (buffer_manager->command_buffer);

	buffer_manager->command_buffer = nullptr;
}

#include "render.h"

#include "block.h"
#include "buffers/buffer.h"
#include "context.h"
#include "drawable.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <glm/glm.hpp>

#include "cameras/camera.h"
#include "editor/editor.h"
#include "inputs/input.h"
#include "pipelines/pipeline.h"
#include "shaders/shader.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <imgui_internal.h>

RenderManager::RenderManager (
	SDL_GPUDevice* device, SDL_Window* window,
	std::shared_ptr<ShaderManager> shader_manager,
	std::shared_ptr<PipelineManager> pipeline_manager,
	std::shared_ptr<BufferManager> buffer_manager,
	std::shared_ptr<AssetManager> asset_manager,
	std::shared_ptr<EditorManager> editor_manager,
	std::shared_ptr<FrameManager> frame_manager,
	std::shared_ptr<ResourceManager> resource_manager
)
	: editor_manager (std::move (editor_manager)),
	  shader_manager (std::move (shader_manager)),
	  pipeline_manager (std::move (pipeline_manager)),
	  buffer_manager (std::move (buffer_manager)),
	  asset_manager (std::move (asset_manager)),
	  frame_manager (std::move (frame_manager)),
	  resource_manager (std::move (resource_manager)), device (device),
	  window (window) {
	assert (device);
	assert (window);

	assert (this->shader_manager);
	assert (this->pipeline_manager);
	assert (this->buffer_manager);
	assert (this->asset_manager);

	int width = this->resource_manager->viewport_target.width;
	int height = this->resource_manager->viewport_target.height;

	SDL_GetWindowSize (window, &width, &height);
	assert (width > 0 && height > 0);

	load_shaders ();
	create_gbuffer_textures (width, height);
	create_depth_texture ();
	create_viewport_texture (
		this->resource_manager->viewport_target.width,
		this->resource_manager->viewport_target.height
	);
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
			.num_uniform_buffers = 1
		},
		ShaderConfig{
			.path = shader_base + "bin/gbuffer.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 1
		},
		"geometry"
	);

	shader_manager->load_shader (
		ShaderConfig{
			.path = shader_base + "bin/screen.vert.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_VERTEX,
			.num_uniform_buffers = 1
		},
		ShaderConfig{
			.path = shader_base + "bin/lighting.frag.metallib",
			.entrypoint = "main0",
			.format = SDL_GPU_SHADERFORMAT_METALLIB,
			.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
			.num_uniform_buffers = 1,
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

void RenderManager::create_depth_texture () const {
	if (buffer_manager->depth_texture)
		return;

	int width = resource_manager->viewport_target.width;
	int height = resource_manager->viewport_target.height;

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

void RenderManager::create_viewport_texture (
	const int width, const int height
) {
	if (width <= 0 || height <= 0)
		return;

	auto& viewport_target = resource_manager->viewport_target;

	if (viewport_target.valid () && viewport_target.width == width
		&& viewport_target.height == height)
		return;

	for (auto& texture : viewport_target.textures) {
		if (texture) {
			SDL_ReleaseGPUTexture (device, texture);
			texture = nullptr;
		}
	}

	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.width = width;
	info.height = height;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
				 | SDL_GPU_TEXTUREUSAGE_SAMPLER;

	for (int i = 0; i < Target::BufferCount; ++i) {
		viewport_target.textures[i] = SDL_CreateGPUTexture (device, &info);
		assert (viewport_target.textures[i]);
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

	render_graph.add_pass (RenderPasses::UIPass);
	assert (render_graph.get_render_pass ("ui_pass"));
	assert (RenderPasses::DeferredPass.execute);
}

void RenderManager::resize (const int new_width, const int new_height) {
	resource_manager->viewport_target.width = new_width;
	resource_manager->viewport_target.height = new_height;

	SDL_WaitForGPUIdle (device);

	if (buffer_manager->depth_texture) {
		SDL_ReleaseGPUTexture (device, buffer_manager->depth_texture);
		buffer_manager->depth_texture = nullptr;
		assert (buffer_manager);
	}

	destroy_gbuffer_textures ();
	create_gbuffer_textures (
		resource_manager->viewport_target.width,
		resource_manager->viewport_target.height
	);
	create_depth_texture ();
	create_viewport_texture (
		resource_manager->viewport_target.width,
		resource_manager->viewport_target.height
	);
}

void RenderManager::acquire_swap_chain () {
	SDL_GPUTexture* swap_chain_texture = nullptr;
	Uint32 width = static_cast<Uint32> (
		resource_manager->viewport_target.width
	);
	Uint32 height = static_cast<Uint32> (
		resource_manager->viewport_target.height
	);

	if (!SDL_WaitAndAcquireGPUSwapchainTexture (
			buffer_manager->command_buffer, window, &swap_chain_texture, &width,
			&height
		)) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER,
			"Failed to acquire swap chain texture (window may be minimized)"
		);
		return;
	}

	buffer_manager->swap_chain_texture = swap_chain_texture;
}

void RenderManager::prepare_drawables (std::vector<Drawable>& drawables) const {
	for (Drawable& drawable : drawables) {
		assert (drawable.mesh);
		assert (&drawable.instance_blocks);

		// --- Instance buffer ---
		if (!drawable.instance_blocks.empty ()) {
			drawable.instance_buffer
				= buffer_manager->get_or_create_instance_buffer (drawable);
		} else {
			std::vector<Block> instance_blocks;

			Block block{};
			write_vec4 (
				block, 0, glm::vec4 (drawable.transform.position, 1.0f)
			);
			write_vec4 (
				block, 1,
				glm::vec4 (
					drawable.transform.rotation.x,
					drawable.transform.rotation.y,
					drawable.transform.rotation.z, drawable.transform.rotation.w
				)
			);
			write_vec4 (block, 2, glm::vec4 (drawable.transform.scale, 0.0f));
			write_vec4 (block, 3, glm::vec4 (0.0f));

			instance_blocks.push_back (block);
			drawable.instance_blocks = instance_blocks;
			drawable.instance_buffer
				= buffer_manager->get_or_create_instance_buffer (drawable);
		}

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

void RenderManager::render (
	RenderState& render_state, const KeyboardInput& key_board_input,
	MouseInput& mouse_input, float delta_time
) {
	assert (&render_state);

	resource_manager->resize_viewport (
		device, editor_manager->viewport_state.width,
		editor_manager->viewport_state.height
	);

	buffer_manager->command_buffer = SDL_AcquireGPUCommandBuffer (device);
	assert (buffer_manager->command_buffer);

	acquire_swap_chain ();
	assert (buffer_manager->swap_chain_texture);

	prepare_drawables (render_state.drawables);

	RenderContext render_context{
		.camera_manager = render_state.scene->camera_manager.get (),
		.pipeline_manager = pipeline_manager.get (),
		.buffer_manager = buffer_manager.get (),
		.shader_manager = shader_manager.get (),
		.frame_manager = frame_manager.get (),
		.resource_manager = resource_manager.get (),
		.drawables = &render_state.drawables,
		.width = resource_manager->viewport_target.width,
		.height = resource_manager->viewport_target.height,
		.time = delta_time
	};

	ImGui_ImplSDLGPU3_NewFrame ();
	ImGui_ImplSDL3_NewFrame ();
	ImGui::NewFrame ();

	editor_manager->create_ui (*render_context.resource_manager, render_state);

	const ImGuiIO& io = ImGui::GetIO ();

	const bool allow_camera_input
		= (editor_manager->editor_mode == Running)
		  || (editor_manager->editor_mode == Editing
			  && editor_manager->editor_state.viewport_hovered
			  && !io.WantCaptureMouse);

	if (allow_camera_input) {
		render_state.scene->camera_manager->update_camera_position (
			delta_time, key_board_input.keys
		);
		render_state.scene->camera_manager->update_camera_look (&mouse_input);
	}

	ImGui::Render ();

	render_graph.execute_all (render_context);

	SDL_SubmitGPUCommandBuffer (buffer_manager->command_buffer);

	resource_manager->viewport_target.swap ();

	buffer_manager->command_buffer = nullptr;
}

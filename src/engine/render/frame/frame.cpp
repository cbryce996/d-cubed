#include "frame.h"

#include "render/buffers/buffer.h"
#include "render/drawable.h"
#include "render/mesh.h"
#include "render/pass/pass.h"
#include "render/pipelines/pipeline.h"

SDL_GPURenderPass* FrameManager::begin_render_pass (
	const RenderPassInstance& render_pass_instance,
	const BufferManager& buffer_manager
) const {
	assert (render_pass_instance.type != RenderPassType::Setup);

	assert (buffer_manager.command_buffer);
	assert (buffer_manager.depth_texture);
	assert (buffer_manager.swap_chain_texture);

	assert (!render_pass_instance.color_targets.empty ());

	for (const auto color_target : render_pass_instance.color_targets) {
		assert (color_target);
	}

	std::vector<SDL_GPUColorTargetInfo> color_target_infos;
	color_target_infos.reserve (render_pass_instance.color_targets.size ());

	for (SDL_GPUTexture* texture : render_pass_instance.color_targets) {
		SDL_GPUColorTargetInfo info{};
		info.texture = texture;
		info.mip_level = 0;
		info.layer_or_depth_plane = 0;
		info.clear_color = render_pass_instance.clear_color;
		info.load_op = SDL_GPU_LOADOP_CLEAR;
		info.store_op = SDL_GPU_STOREOP_STORE;
		info.resolve_texture = nullptr;
		info.cycle = true;
		info.cycle_resolve_texture = false;
		color_target_infos.push_back (info);
	}

	if (render_pass_instance.depth_target) {
		SDL_GPUDepthStencilTargetInfo depth_target_info{};
		depth_target_info.texture = render_pass_instance.depth_target;
		depth_target_info.clear_depth = render_pass_instance.clear_depth ? 1.0f
																		 : 0.0f;
		depth_target_info.load_op = render_pass_instance.clear_depth
										? SDL_GPU_LOADOP_CLEAR
										: SDL_GPU_LOADOP_LOAD;
		depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
		depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
		depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
		depth_target_info.cycle = true;

		SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass (
			buffer_manager.command_buffer, color_target_infos.data (),
			static_cast<uint32_t> (color_target_infos.size ()),
			&depth_target_info
		);
		assert (render_pass);
		return render_pass;
	}

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass (
		buffer_manager.command_buffer, color_target_infos.data (),
		static_cast<uint32_t> (color_target_infos.size ()), nullptr
	);
	assert (render_pass);
	return render_pass;
}

void FrameManager::set_viewport (
	SDL_GPURenderPass* current_render_pass, int width, int height
) {
	assert (current_render_pass);

	SDL_GPUViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.w = static_cast<float> (width);
	viewport.h = static_cast<float> (height);
	viewport.min_depth = 0.0f;
	viewport.max_depth = 1.0f;
	SDL_SetGPUViewport (current_render_pass, &viewport);
}

void FrameManager::draw_mesh (
	const Pipeline& pipeline, const Buffer& vertex_buffer,
	const Buffer& instance_buffer, const Buffer& index_buffer,
	const Drawable& drawable, SDL_GPURenderPass& render_pass
) {
	assert (&pipeline);
	assert (&vertex_buffer);
	assert (&instance_buffer);
	assert (&drawable);

	// Create render pass and bind to pipeline
	SDL_BindGPUGraphicsPipeline (&render_pass, pipeline.pipeline);

	// Bing vertex buffer with instance
	const SDL_GPUBufferBinding vertex_bindings[2] = {
		{vertex_buffer.gpu_buffer.buffer, 0},
		{instance_buffer.gpu_buffer.buffer, 0},
	};

	SDL_BindGPUVertexBuffers (&render_pass, 0, vertex_bindings, 2);

	const SDL_GPUBufferBinding index_bindings[1] = {
		{index_buffer.gpu_buffer.buffer, 0}
	};

	SDL_BindGPUIndexBuffer (
		&render_pass, index_bindings, SDL_GPU_INDEXELEMENTSIZE_32BIT
	);

	SDL_DrawGPUIndexedPrimitives (
		&render_pass, static_cast<Uint32> (drawable.mesh->gpu_indices.size ()),
		static_cast<Uint32> (drawable.instance_batch->blocks.size ()), 0, 0, 0
	);
}

void FrameManager::draw_screen (
	const Pipeline& pipeline, BufferManager& buffer_manager,
	SDL_GPURenderPass& render_pass
) {
	assert (&pipeline);

	SDL_BindGPUGraphicsPipeline (&render_pass, pipeline.pipeline);

	SDL_GPUTextureSamplerBinding samplers[3];
	samplers[0].texture = buffer_manager.g_position_texture;
	samplers[0].sampler = buffer_manager.linear_sampler;
	samplers[1].texture = buffer_manager.g_normal_texture;
	samplers[1].sampler = buffer_manager.linear_sampler;
	samplers[2].texture = buffer_manager.g_albedo_texture;
	samplers[2].sampler = buffer_manager.linear_sampler;
	SDL_BindGPUFragmentSamplers (&render_pass, 0, samplers, 3);

	SDL_DrawGPUPrimitives (&render_pass, 3, 1, 0, 0);
}
#include "frame.h"

#include "mesh/mesh.h"
#include "render/buffers/buffer.h"
#include "render/drawable.h"
#include "render/pass/pass.h"
#include "render/pipelines/pipeline.h"

#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

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

	assert (render_pass_instance.color_targets.size () <= 8);

	std::vector<SDL_GPUColorTargetInfo> color_target_infos;
	color_target_infos.reserve (render_pass_instance.color_targets.size ());

	for (SDL_GPUTexture* texture : render_pass_instance.color_targets) {
		assert (texture);

		SDL_GPUColorTargetInfo info{};
		info.texture = texture;
		info.mip_level = 0;
		info.layer_or_depth_plane = 0;
		info.clear_color = render_pass_instance.clear_color;
		info.load_op = render_pass_instance.load_op;
		info.store_op = SDL_GPU_STOREOP_STORE;
		info.resolve_texture = nullptr;
		info.cycle = (info.load_op == SDL_GPU_LOADOP_CLEAR);
		info.cycle_resolve_texture = false;

		color_target_infos.push_back (info);
	}

	assert (!color_target_infos.empty ());

	if (render_pass_instance.depth_target) {
		assert (render_pass_instance.depth_target);

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
	assert (width > 0);
	assert (height > 0);

	SDL_GPUViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.w = static_cast<float> (width);
	viewport.h = static_cast<float> (height);
	viewport.min_depth = 0.0f;
	viewport.max_depth = 1.0f;
	SDL_SetGPUViewport (current_render_pass, &viewport);

	SDL_Rect scissor{};
	scissor.x = 0;
	scissor.y = 0;
	scissor.w = width;
	scissor.h = height;
	SDL_SetGPUScissor (current_render_pass, &scissor);
}

void FrameManager::draw_mesh (
	const Pipeline& pipeline, BufferManager& buffer_manager, Drawable& drawable,
	SDL_GPURenderPass& render_pass
) {
	assert (drawable.mesh);
	assert (drawable.material);

	assert (pipeline.pipeline);

	assert (!drawable.mesh->gpu_state.vertices.empty ());
	assert (!drawable.mesh->gpu_state.indices.empty ());
	assert (!drawable.instance_blocks.empty ());

	SDL_BindGPUGraphicsPipeline (&render_pass, pipeline.pipeline);

	const Buffer* vertex_buffer = buffer_manager.get_or_create_vertex_buffer (
		*drawable.mesh
	);
	const Buffer* index_buffer = buffer_manager.get_or_create_index_buffer (
		*drawable.mesh
	);
	const Buffer* instance_buffer
		= buffer_manager.get_or_create_instance_buffer (drawable);

	assert (vertex_buffer);
	assert (index_buffer);
	assert (instance_buffer);

	assert (vertex_buffer->gpu_buffer.buffer);
	assert (index_buffer->gpu_buffer.buffer);
	assert (instance_buffer->gpu_buffer.buffer);

	assert (vertex_buffer->size > 0);
	assert (index_buffer->size > 0);
	assert (instance_buffer->size > 0);

	assert (vertex_buffer->size % ALIGNMENT == 0);
	assert (instance_buffer->size % ALIGNMENT == 0);

	const SDL_GPUBufferBinding vertex_bindings[2] = {
		{vertex_buffer->gpu_buffer.buffer, 0},
		{instance_buffer->gpu_buffer.buffer, 0},
	};

	SDL_BindGPUVertexBuffers (&render_pass, 0, vertex_bindings, 2);

	const SDL_GPUBufferBinding index_bindings[1] = {
		{index_buffer->gpu_buffer.buffer, 0}
	};

	SDL_BindGPUIndexBuffer (
		&render_pass, index_bindings, SDL_GPU_INDEXELEMENTSIZE_32BIT
	);

	assert (drawable.mesh->gpu_state.indices.size () <= UINT32_MAX);
	assert (drawable.instance_blocks.size () <= UINT32_MAX);

	SDL_DrawGPUIndexedPrimitives (
		&render_pass,
		static_cast<Uint32> (drawable.mesh->gpu_state.indices.size ()),
		static_cast<Uint32> (drawable.instance_blocks.size ()), 0, 0, 0
	);
}

void FrameManager::draw_screen (
	const Pipeline& pipeline, BufferManager& buffer_manager,
	SDL_GPURenderPass& render_pass
) {
	assert (pipeline.pipeline);

	assert (buffer_manager.g_position_texture);
	assert (buffer_manager.g_normal_texture);
	assert (buffer_manager.g_albedo_texture);
	assert (buffer_manager.linear_sampler);

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

void FrameManager::draw_ui (
	const BufferManager& buffer_manager, SDL_GPURenderPass& render_pass
) {
	ImDrawData* draw_data = ImGui::GetDrawData ();
	if (!draw_data || draw_data->TotalVtxCount == 0)
		return;

	ImGui_ImplSDLGPU3_RenderDrawData (
		draw_data, buffer_manager.command_buffer, &render_pass
	);
}

void FrameManager::prepare_ui (const BufferManager& buffer_manager) {
	assert (buffer_manager.command_buffer);

	ImDrawData* draw_data = ImGui::GetDrawData ();
	if (!draw_data || draw_data->TotalVtxCount == 0)
		return;

	ImGui_ImplSDLGPU3_PrepareDrawData (
		draw_data, buffer_manager.command_buffer
	);
}

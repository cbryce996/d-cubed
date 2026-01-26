
#include "pass.h"
#include "cameras/camera.h"
#include "render/buffers/buffer.h"
#include "render/context.h"
#include "render/drawable.h"
#include "render/frame/frame.h"
#include "render/pipelines/pipeline.h"
#include "render/render.h"
#include "utils.h"

namespace RenderPasses {
RenderPassInstance UniformPass = {
	.name = "uniform_pass",
	.type = RenderPassType::Setup,
	.execute = [] (RenderContext& render_context,
				   RenderPassInstance& render_pass_instance) {
		assert (render_context.camera_manager->get_active_camera ());

		const Camera* active_camera
			= render_context.camera_manager->get_active_camera ();
		const float aspect_ratio = static_cast<float> (render_context.width)
								   / static_cast<float> (render_context.height);
		const glm::vec3 light_pos_world = active_camera->transform.position;

		const glm::mat4 view_projection
			= CameraManager::compute_view_projection (
				*active_camera, aspect_ratio
			);

		Block view_uniform_block{};
		write_vec4 (view_uniform_block, 0, view_projection[0]);
		write_vec4 (view_uniform_block, 1, view_projection[1]);
		write_vec4 (view_uniform_block, 2, view_projection[2]);
		write_vec4 (view_uniform_block, 3, view_projection[3]);

		Block global_uniform_block{};
		write_vec4 (global_uniform_block, 0, glm::vec4 (light_pos_world, 1.0f));
		write_vec4 (
			global_uniform_block, 1,
			glm::vec4 (render_context.time, 0.0f, 0.0f, 0.0f)
		);
		write_vec4 (
			global_uniform_block, 2,
			glm::vec4 (active_camera->transform.position, 0.0f)
		);

		std::vector<UniformBinding> uniform_bindings;

		UniformBinding view_uniform_binding{};
		view_uniform_binding.data = view_uniform_block.data;
		view_uniform_binding.slot = 0;
		view_uniform_binding.size = sizeof (Block);
		view_uniform_binding.stage = ShaderStage::Vertex;
		uniform_bindings.push_back (view_uniform_binding);

		UniformBinding global_uniform_binding{};
		global_uniform_binding.data = global_uniform_block.data;
		global_uniform_binding.slot = 1;
		global_uniform_binding.size = sizeof (Block);
		global_uniform_binding.stage = ShaderStage::Both;
		uniform_bindings.push_back (global_uniform_binding);

		render_context.buffer_manager->push_uniforms (uniform_bindings);
	}
};

RenderPassInstance GeometryPass = {
	.name = "geometry_pass",
	.type = RenderPassType::Geometry,
	.state = {
		.depth_compare = SDL_GPU_COMPAREOP_LESS,
		.depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.color_formats
		= {SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
		   SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
		   SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM},
		.has_depth_stencil_target = true,
	},
	.clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
	.clear_depth = true,
	.execute = [] (
				   RenderContext& render_context,
				   RenderPassInstance& render_pass_instance
			   ) {
		assert (render_context.buffer_manager->g_position_texture);
		assert (render_context.buffer_manager->g_normal_texture);
		assert (render_context.buffer_manager->g_albedo_texture);
		assert (render_context.buffer_manager->depth_texture);

		render_pass_instance.depth_target = render_context.buffer_manager->depth_texture;

		render_pass_instance.color_targets = {
			render_context.buffer_manager->g_position_texture,
			render_context.buffer_manager->g_normal_texture, render_context.buffer_manager->g_albedo_texture
		};

		render_context.render_pass = render_context.frame_manager->begin_render_pass (render_pass_instance, *render_context.buffer_manager);
		render_context.frame_manager->set_viewport (render_context.render_pass, render_context.width, render_context.height);

		for (Drawable& drawable : *render_context.drawables) {
			const PipelineState pipeline_state{
				.render_pass_state = render_pass_instance.state,
				.material_state = drawable.material->state,
			};

			const Pipeline* pipeline = render_context.pipeline_manager
										   ->get_or_create (pipeline_state);

			render_context.frame_manager->draw_mesh (
				*pipeline, *render_context.buffer_manager, drawable,
				*render_context.render_pass
			);
		}

		SDL_EndGPURenderPass (render_context.render_pass);
	}
};

RenderPassInstance DeferredPass = {
	RenderPassInstance {
		.name = "deferred_pass",
		.type = RenderPassType::Lighting,
			.state = {
			.color_formats = {SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM},
			.has_depth_stencil_target = false,
		},
		.dependencies = {"geometry_pass"},
		.depth_target = nullptr,
		.clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
		.clear_depth = false,
		.execute = [] (
									RenderContext& render_context,
									RenderPassInstance& render_pass_instance
								) {
			assert (render_context.buffer_manager->swap_chain_texture);

			render_pass_instance.color_targets = {
				render_context.buffer_manager->swap_chain_texture
			};

			render_context.render_pass = render_context.frame_manager->begin_render_pass (render_pass_instance, *render_context.buffer_manager);
			render_context.frame_manager->set_viewport (render_context.render_pass, render_context.width, render_context.height);

			const PipelineState pipeline_state{
				.render_pass_state = render_pass_instance.state,
				.material_state = Materials::Deferred.state,
			};

			const Pipeline* pipeline
				= render_context.pipeline_manager->get_or_create (pipeline_state);

			render_context.frame_manager->draw_screen (*pipeline, *render_context.buffer_manager, *render_context.render_pass);

			SDL_EndGPURenderPass (render_context.render_pass);
		}
	}
};
};
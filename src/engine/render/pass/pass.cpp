
#include "pass.h"
#include "core/camera/camera.h"
#include "render/buffers/buffer.h"
#include "render/context.h"
#include "render/drawable.h"
#include "render/frame/frame.h"
#include "render/pipelines/pipeline.h"
#include "render/render.h"
#include "render/textures/registry.h"
#include "utils.h"

namespace RenderPasses {
RenderPassInstance UniformPass = {
	.name = "uniform_pass",
	.type = RenderPassType::Setup,
	.depth_target = false,
	.execute = [] (const RenderContext& render_context,
				   RenderPassInstance& render_pass_instance) {
		assert (render_context.camera_manager->get_active_camera ());

		const Camera* active_camera
			= render_context.camera_manager->get_active_camera ();
		const glm::vec3 light_pos_world = active_camera->transform.position;

		const glm::mat4 view_projection
			= CameraManager::compute_view_projection (
				*active_camera,
				render_context.texture_registry->viewport.aspect_ratio ()
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
		global_uniform_binding.slot = 0;
		global_uniform_binding.size = sizeof (Block);
		global_uniform_binding.stage = ShaderStage::Fragment;
		uniform_bindings.push_back (global_uniform_binding);

		render_context.buffer_manager->push_uniforms (uniform_bindings);
	}
};

RenderPassInstance GeometryPass = {
	.name = "geometry_pass",
	.type = RenderPassType::Geometry,
	.dependencies = {"uniform_pass"},
	.state = {
		.depth_compare = CompareOp::Less,
		.depth_format = TextureFormat::D32F,
		.depth_stencil_format = TextureFormat::D32F,
		.color_formats
		= {TextureFormat::RGBA16F,
		   TextureFormat::RGBA16F,
		   TextureFormat::RGBA8},
		.has_depth_stencil_target = true,
	},
	.load_op = LoadOp::Clear,
	.depth_target = true,
	.clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
	.clear_depth = true,
	.execute = [] (
				   RenderContext& render_context,
				   RenderPassInstance& render_pass_instance
			   ) {
		assert (render_context.buffer_manager->depth_texture);

		render_pass_instance.target_textures = {
			render_context.texture_registry->gbuffer_position.read(),
			render_context.texture_registry->gbuffer_normal.read(),
			render_context.texture_registry->gbuffer_albedo.read()
		};

		render_pass_instance.sampled_textures.clear();

		render_context.render_pass = render_context.frame_manager->begin_render_pass (render_pass_instance, render_context);

		render_context.frame_manager->draw_mesh (
			render_context, render_pass_instance.state
		);

		SDL_EndGPURenderPass (render_context.render_pass);
	}
};

RenderPassInstance DeferredPass = {
	RenderPassInstance {
		.name = "deferred_pass",
		.type = RenderPassType::Lighting,
		.state = {
			.color_formats = {TextureFormat::BGRA8},
			.has_depth_stencil_target = false,
		},
		.dependencies = {"geometry_pass"},
		.load_op = LoadOp::Clear,
		.depth_target = false,
		.clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
		.clear_depth = false,
		.execute = [] (
			RenderContext& render_context,
			RenderPassInstance& render_pass_instance
		) {
			assert(render_context.texture_registry);
			assert(render_context.frame_manager);
			assert(render_context.buffer_manager);

			if (!render_context.texture_registry->viewport.valid()) {
				return;
			}

			render_pass_instance.target_textures = {
				render_context.texture_registry->viewport.write()
			};

			render_pass_instance.sampled_textures = {
				render_context.texture_registry->gbuffer_position.read(),
				render_context.texture_registry->gbuffer_normal.read(),
				render_context.texture_registry->gbuffer_albedo.read()
			};

			render_context.render_pass =
				render_context.frame_manager->begin_render_pass(
					render_pass_instance,
					render_context
				);

			render_context.frame_manager->draw_screen(
				render_context,
				render_pass_instance,
				render_pass_instance.state
			);

			SDL_EndGPURenderPass(render_context.render_pass);
		}
	}
};

RenderPassInstance UIPass = {
	RenderPassInstance {
		.name = "ui_pass",
		.type = RenderPassType::UI,
		.state = {
			.color_formats = {TextureFormat::BGRA8},
			.has_depth_stencil_target = false,
		},
		.dependencies = { "deferred_pass" },
		.load_op = LoadOp::Clear,
		.swap_chain_target = true,
		.depth_target = false,
		.clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
		.clear_depth = false,
		.execute = [] (
			RenderContext& render_context,
			const RenderPassInstance& render_pass_instance
		) {

			render_context.frame_manager->prepare_ui (*render_context.buffer_manager);

			render_context.render_pass =
				render_context.frame_manager->begin_render_pass(
					render_pass_instance,
					render_context
				);

			render_context.frame_manager->draw_ui (
				*render_context.buffer_manager, *render_context.render_pass
			);

			SDL_EndGPURenderPass(render_context.render_pass);
		}
	}
};

};
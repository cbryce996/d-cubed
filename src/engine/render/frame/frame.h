#ifndef FRAME_H
#define FRAME_H

#include <SDL3/SDL.h>

#include "render/pass/pass.h"
struct RenderContext;
class TextureRegistry;
struct Drawable;
struct Buffer;
struct Pipeline;
class BufferManager;
struct RenderPassInstance;

class FrameManager {
  public:
	[[nodiscard]] SDL_GPURenderPass* begin_render_pass (
		const RenderPassInstance& render_pass_instance,
		const BufferManager& buffer_manager
	) const;

	void set_viewport (
		SDL_GPURenderPass* current_render_pass, int width, int height
	);
	void draw_mesh (
		const RenderContext& render_context,
		const RenderPassState& render_pass_state
	);
	void draw_screen (
		const RenderContext& render_context,
		const RenderPassInstance& render_pass_instance,
		const RenderPassState& render_pass_state
	);
	void draw_ui (
		const BufferManager& buffer_manager, SDL_GPURenderPass& render_pass
	);
	void prepare_ui (const BufferManager& buffer_manager);
};

#endif // FRAME_H

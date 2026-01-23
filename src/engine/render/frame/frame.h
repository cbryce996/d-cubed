#ifndef FRAME_H
#define FRAME_H

#include <SDL3/SDL.h>

struct Drawable;
struct Buffer;
struct Pipeline;
class BufferManager;
struct RenderPassInstance;

class FrameManager {
  public:
	SDL_GPURenderPass* begin_render_pass (
		const RenderPassInstance& render_pass_instance,
		const BufferManager& buffer_manager
	) const;

	void set_viewport (
		SDL_GPURenderPass* current_render_pass, int width, int height
	);
	void draw_mesh (
		const Pipeline& pipeline, const Buffer& vertex_buffer,
		const Buffer& instance_buffer, const Buffer& index_buffer,
		const Drawable& drawable, SDL_GPURenderPass& render_pass
	);
	void draw_screen (
		const Pipeline& pipeline, BufferManager& buffer_manager,
		SDL_GPURenderPass& render_pass
	);
};

#endif // FRAME_H

#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>

#include "buffer.h"
#include "camera.h"
#include "drawable.h"
#include "graph.h"
#include "pipeline.h"
#include "shader.h"

struct RenderState {
	std::vector<SDL_FRect> item_rects;
	std::vector<SDL_FRect> crafting_rects;
	SDL_FRect player_rect{};
};

struct RenderContext {
	CameraManager* camera_manager = nullptr;
	PipelineManager* pipeline_manager = nullptr;
	BufferManager* buffer_manager = nullptr;
	ShaderManager* shader_manager = nullptr;

	std::vector<Drawable>* geometry_drawables = nullptr;
	SDL_GPURenderPass* render_pass = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;
};

class RenderManager {
   public:
	RenderManager(
		SDL_GPUDevice* device,
		SDL_Window* window,
		std::shared_ptr<ShaderManager> shader_manager,
		std::shared_ptr<PipelineManager> pipeline_manager,
		std::shared_ptr<BufferManager> buffer_manager,
		std::shared_ptr<CameraManager> camera_manager
	);
	~RenderManager();

	Uint32 width = 1280;
	Uint32 height = 720;

	std::shared_ptr<ShaderManager> shader_manager;
	std::shared_ptr<PipelineManager> pipeline_manager;
	std::shared_ptr<BufferManager> buffer_manager;
	std::shared_ptr<CameraManager> camera_manager;

	void render();

	void create_swap_chain_texture();
	void create_depth_texture() const;
	void draw_mesh(
		const Pipeline *pipeline,
		const Buffer *buffer,
		const Mesh *mesh,
		const Uniform &uniform
	);

	void prepare_drawables(std::vector<Drawable> &drawables) const;

	void set_viewport(SDL_GPURenderPass *current_render_pass);

	[[nodiscard]] SDL_GPURenderPass* create_render_pass() const;

	private:
		SDL_GPUDevice* device = nullptr;
		SDL_Window* window = nullptr;
		SDL_GPURenderPass* current_render_pass = nullptr;

		RenderGraph render_graph;
};

#endif	// RENDERER_H

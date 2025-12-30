#ifndef RENDERER_H
#define RENDERER_H

#include "assets/asset.h"
#include "drawable.h"
#include "graph.h"
#include "memory.h"
#include "pipeline.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <vector>

class CameraManager;
enum class ShaderStage : uint8_t {
	Vertex = 1 << 0,
	Fragment = 1 << 1,
	Both = Vertex | Fragment
};

struct UniformBinding {
	uint32_t slot;
	const void* data;
	uint32_t size;
	ShaderStage stage;
};

struct Uniform : Collection {};

struct RenderState {
	std::vector<Drawable> drawables;
};

struct RenderContext {
	CameraManager* camera_manager = nullptr;
	PipelineManager* pipeline_manager = nullptr;
	BufferManager* buffer_manager = nullptr;
	ShaderManager* shader_manager = nullptr;

	std::vector<Drawable>* drawables = nullptr;
	SDL_GPURenderPass* render_pass = nullptr;

	int width = 0;
	int height = 0;

	float time = 0.0f;
};

class RenderManager {
  public:
	RenderManager (
		SDL_GPUDevice* device, SDL_Window* window,
		std::shared_ptr<ShaderManager> shader_manager,
		std::shared_ptr<PipelineManager> pipeline_manager,
		std::shared_ptr<BufferManager> buffer_manager,
		std::shared_ptr<CameraManager> camera_manager,
		std::shared_ptr<AssetManager> asset_manager
	);
	~RenderManager ();
	void setup_render_graph ();
	void resize (int new_width, int new_height);
	void acquire_swap_chain ();

	int width = 1920;
	int height = 1080;

	std::shared_ptr<ShaderManager> shader_manager;
	std::shared_ptr<PipelineManager> pipeline_manager;
	std::shared_ptr<BufferManager> buffer_manager;
	std::shared_ptr<CameraManager> camera_manager;
	std::shared_ptr<AssetManager> asset_manager;

	void render (RenderState* render_state, float time);

	void create_depth_texture () const;
	void draw (
		const Pipeline* pipeline, const Buffer* vertex_buffer,
		const Buffer* instance_buffer, const Drawable* drawable,
		const std::vector<UniformBinding>* uniform_bindings
	);

	void prepare_drawables (std::vector<Drawable>& drawables) const;

	void set_viewport (SDL_GPURenderPass* current_render_pass);

	[[nodiscard]] SDL_GPURenderPass* create_render_pass () const;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	SDL_GPURenderPass* current_render_pass = nullptr;

	RenderGraph render_graph;
};

#endif // RENDERER_H

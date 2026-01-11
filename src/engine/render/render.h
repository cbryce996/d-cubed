#ifndef RENDERER_H
#define RENDERER_H

#include "assets/asset.h"
#include "cameras/camera.h"
#include "drawable.h"
#include "graph/graph.h"
#include "pipelines/pipeline.h"
#include "shaders/shader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <vector>

enum class ShaderStage : uint8_t {
	Vertex = 1 << 0,
	Fragment = 1 << 1,
	Both = Vertex | Fragment
};

struct UniformBinding {
	std::string name;
	uint32_t slot;
	const void* data;
	uint32_t size;
	ShaderStage stage;
};

struct RenderState {
	std::vector<Drawable> drawables;
};

struct RenderPassConfig {
	std::vector<SDL_GPUTexture*> color_targets;
	SDL_GPUTexture* depth_target; // nullable
	SDL_FColor clear_color;
	bool clear_depth;
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
	void load_pipelines () const;
	void setup_render_graph ();
	void resize (int new_width, int new_height);
	void acquire_swap_chain ();
	void load_shaders () const;

	int width = 1920;
	int height = 1080;

	std::shared_ptr<ShaderManager> shader_manager;
	std::shared_ptr<PipelineManager> pipeline_manager;
	std::shared_ptr<BufferManager> buffer_manager;
	std::shared_ptr<CameraManager> camera_manager;
	std::shared_ptr<AssetManager> asset_manager;

	void render (RenderState* render_state, float time);

	void create_depth_texture () const;
	void create_gbuffer_textures (int width, int height);
	void destroy_gbuffer_textures ();

	void draw_mesh (
		const Pipeline* pipeline, const Buffer* vertex_buffer,
		const Buffer* instance_buffer, const Buffer* index_buffer,
		const Drawable* drawable
	);

	void draw_screen (const Pipeline* pipeline);
	void
	push_uniform_bindings (std::vector<UniformBinding>& uniform_bindings) const;

	void prepare_drawables (std::vector<Drawable>& drawables) const;

	void set_viewport (SDL_GPURenderPass* current_render_pass);

	[[nodiscard]] SDL_GPURenderPass*
	create_render_pass (const RenderPassConfig& render_pass_config) const;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	SDL_GPURenderPass* current_render_pass = nullptr;

	RenderGraph render_graph;
};

#endif // RENDERER_H

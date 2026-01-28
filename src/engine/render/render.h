#ifndef RENDERER_H
#define RENDERER_H

#include "drawable.h"
#include "graph/graph.h"
#include "scene.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <vector>

class AssetManager;
class CameraManager;
class BufferManager;
class PipelineManager;
class ShaderManager;

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
	Scene* scene;
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
	void load_shaders () const;

	int width = 1920;
	int height = 1080;

	int viewport_w = 0;
	int viewport_h = 0;

	std::shared_ptr<ShaderManager> shader_manager;
	std::shared_ptr<PipelineManager> pipeline_manager;
	std::shared_ptr<BufferManager> buffer_manager;
	std::shared_ptr<CameraManager> camera_manager;
	std::shared_ptr<AssetManager> asset_manager;

	void render (RenderState* render_state, float time);

	void create_depth_texture () const;
	void create_viewport_texture (int w, int h);
	void create_gbuffer_textures (int width, int height) const;
	void destroy_gbuffer_textures () const;

	void prepare_drawables (std::vector<Drawable>& drawables) const;
	void create_ui ();
	static void layout_ui (ImGuiID dockspace_id);
	void draw_ui (const BufferManager& buffer_manager);

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	SDL_GPURenderPass* current_render_pass = nullptr;

	RenderGraph render_graph;
};

#endif // RENDERER_H

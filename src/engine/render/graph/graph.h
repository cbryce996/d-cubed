#ifndef GRAPH_H
#define GRAPH_H

#include "render/pipelines/pipeline.h"

#include <functional>
#include <string>

struct RenderContext;

enum class RenderPassType { Setup, Geometry, Lighting, PostProcess };

struct RenderPassBindings {
	std::vector<SDL_GPUTexture*> color_targets;
	SDL_GPUTexture* depth_target; // nullable
	SDL_FColor clear_color;
	bool clear_depth;
};

struct RenderPassLayout {
	SDL_GPUCompareOp depth_compare;
	SDL_GPUTextureFormat depth_format;
	SDL_GPUTextureFormat depth_stencil_format;

	std::vector<SDL_GPUTextureFormat> color_formats;
	bool has_depth_stencil_target;

	[[nodiscard]] std::string key() const {
		// TODO: Better hashing of key

		std::string k;
		k += "|depth_cmp=" + std::to_string(depth_compare);

		k += "|has_depth=" + std::to_string(has_depth_stencil_target);
		k += "|depth_fmt=" + std::to_string(depth_format);
		k += "|depth_stencil_fmt=" + std::to_string(depth_stencil_format);

		k += "|colors=" + std::to_string(color_formats.size());
		for (const auto fmt : color_formats) {
			k += "_" + std::to_string(fmt);
		}

		return k;
	}
};

struct RenderPass {
	std::string name;
	RenderPassType type;
	RenderPassLayout render_pass_layout;

	std::function<void (RenderContext&, RenderPass&)> execute;
	std::vector<std::string> dependencies;

	bool completed = false;
};

class RenderGraph {
  public:
	RenderGraph ();
	~RenderGraph ();

	void add_pass (const RenderPass& pass);
	RenderPass* get_render_pass (const std::string& name);
	void execute_all (RenderContext& render_context);

  private:
	std::unordered_map<std::string, RenderPass> render_passes;
	std::vector<std::string> sorted_pass_order;

	void topological_sort ();
};

#endif // GRAPH_H

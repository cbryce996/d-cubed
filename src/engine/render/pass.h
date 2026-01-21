#ifndef PASS_H
#define PASS_H

#include "utils.h"

#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <vector>

struct RenderContext;

enum class RenderPassType { Setup, Geometry, Lighting, PostProcess };

struct RenderPassState {
	SDL_GPUCompareOp depth_compare;
	SDL_GPUTextureFormat depth_format;
	SDL_GPUTextureFormat depth_stencil_format;
	std::vector<SDL_GPUTextureFormat> color_formats;
	bool has_depth_stencil_target;

	bool operator== (const RenderPassState& other) const {
		return depth_compare == other.depth_compare
			   && depth_format == other.depth_format
			   && depth_stencil_format == other.depth_stencil_format
			   && has_depth_stencil_target == other.has_depth_stencil_target
			   && color_formats == other.color_formats;
	}
};

struct RenderPassInstance {
	std::string name;
	RenderPassType type;
	RenderPassState state = {};

	std::vector<SDL_GPUTexture*> color_targets;
	SDL_GPUTexture* depth_target; // nullable
	SDL_FColor clear_color;
	bool clear_depth;

	std::function<void (RenderContext&, RenderPassInstance&)> execute;
	std::vector<std::string> dependencies;

	bool completed = false;
};

template <> struct std::hash<RenderPassState> {
	size_t
	operator() (const RenderPassState& render_pass_state) const noexcept {
		size_t h = 0;

		hash_combine (h, std::hash<int> () (render_pass_state.depth_compare));
		hash_combine (h, std::hash<int> () (render_pass_state.depth_format));
		hash_combine (
			h, std::hash<int> () (render_pass_state.depth_stencil_format)
		);
		hash_combine (
			h, std::hash<bool> () (render_pass_state.has_depth_stencil_target)
		);

		for (const auto color_format : render_pass_state.color_formats) {
			hash_combine (h, std::hash<int> () (color_format));
		}

		return h;
	}
};

#endif // PASS_H

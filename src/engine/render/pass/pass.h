#ifndef PASS_H
#define PASS_H

#include "utils.h"

#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <vector>

#include "core/storage/storage.h"
#include "core/types.h"

enum class LoadOp : uint8_t;
struct RenderContext;

enum class RenderPassType { Setup, Geometry, Lighting, PostProcess, UI };

struct RenderPassState {
	CompareOp depth_compare;
	TextureFormat depth_format;
	TextureFormat depth_stencil_format;
	std::vector<TextureFormat> color_formats;
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

	LoadOp load_op;
	std::vector<Handle> target_textures;
	std::vector<Handle> sampled_textures;
	Color4 clear_color;
	bool swap_chain_target;
	bool depth_target;
	bool clear_depth;

	std::function<void (RenderContext&, RenderPassInstance&)> execute;
	std::vector<std::string> dependencies;

	bool completed = false;
};

template <> struct std::hash<RenderPassState> {
	size_t
	operator() (const RenderPassState& render_pass_state) const noexcept {
		size_t h = 0;

		hash_combine (
			h, std::hash<uint8_t>{}(
				   static_cast<uint8_t> (render_pass_state.depth_compare)
			   )
		);
		hash_combine (
			h, std::hash<uint8_t>{}(
				   static_cast<uint8_t> (render_pass_state.depth_format)
			   )
		);
		hash_combine (
			h, std::hash<uint8_t>{}(
				   static_cast<uint8_t> (render_pass_state.depth_stencil_format)
			   )
		);
		hash_combine (
			h, std::hash<bool>{}(render_pass_state.has_depth_stencil_target)
		);

		for (const auto color_format : render_pass_state.color_formats) {
			hash_combine (
				h, std::hash<uint8_t>{}(static_cast<uint8_t> (color_format))
			);
		}

		return h;
	}
};

namespace RenderPasses {
extern RenderPassInstance UniformPass;
extern RenderPassInstance GeometryPass;
extern RenderPassInstance DeferredPass;
extern RenderPassInstance UIPass;
}

#endif // PASS_H

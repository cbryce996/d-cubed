#ifndef MATERIAL_H
#define MATERIAL_H

#include "utils.h"

#include <SDL3/SDL_gpu.h>
#include <string>

struct MaterialState {
	std::string shader;
	SDL_GPUPrimitiveType primitive_type;
	SDL_GPUCullMode cull_mode;
	SDL_GPUCompareOp compare_op;
	bool enable_depth_test;
	bool enable_depth_write;

	bool operator== (const MaterialState& other) const {
		return shader == other.shader && primitive_type == other.primitive_type
			   && cull_mode == other.cull_mode && compare_op == other.compare_op
			   && enable_depth_test == other.enable_depth_test
			   && enable_depth_write == other.enable_depth_write;
	}
};

struct MaterialInstance {
	std::string name;
	const MaterialState* state;
	glm::vec4 base_color = {1.0f, 1.0f, 1.0f, 1.0f};
};

template <> struct std::hash<MaterialState> {
	size_t operator() (const MaterialState& material) const noexcept {
		size_t h = 0;

		hash_combine (h, std::hash<std::string> () (material.shader));
		hash_combine (h, std::hash<int> () (material.primitive_type));
		hash_combine (h, std::hash<int> () (material.cull_mode));

		hash_combine (h, std::hash<bool> () (material.enable_depth_test));
		hash_combine (h, std::hash<bool> () (material.enable_depth_write));

		if (material.enable_depth_test) {
			hash_combine (h, std::hash<int> () (material.compare_op));
		}

		return h;
	}
};

namespace Materials {
inline static MaterialState GeometryState{
	.shader = "geometry",
	.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
	.cull_mode = SDL_GPU_CULLMODE_BACK,
	.compare_op = SDL_GPU_COMPAREOP_LESS,
	.enable_depth_test = true,
	.enable_depth_write = true
};

inline static MaterialInstance Geometry{
	.name = "geometry",
	.state = &GeometryState,
	.base_color = {1.0f, 1.0f, 1.0f, 1.0f}
};

inline static MaterialState DeferredState{
	.shader = "lighting",
	.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
	.cull_mode = SDL_GPU_CULLMODE_BACK,
	.compare_op = SDL_GPU_COMPAREOP_LESS,
	.enable_depth_test = false,
	.enable_depth_write = false
};

inline static MaterialInstance Deferred{
	.name = "deferred",
	.state = &DeferredState,
	.base_color = {1.0f, 1.0f, 1.0f, 1.0f}
};
}

#endif // MATERIAL_H

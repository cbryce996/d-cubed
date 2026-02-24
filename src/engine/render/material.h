#ifndef MATERIAL_H
#define MATERIAL_H

#include "utils.h"

#include <functional>
#include <string>

#include <glm/vec4.hpp>

#include "core/types.h"

struct MaterialState {
	std::string vertex_shader{};
	std::string fragment_shader{};

	PrimitiveType primitive_type = PrimitiveType::TriangleList;
	CullMode cull_mode = CullMode::Back;
	CompareOp compare_op = CompareOp::Less;

	bool enable_depth_test = true;
	bool enable_depth_write = true;

	bool operator== (const MaterialState& other) const {
		return vertex_shader == other.vertex_shader
			   && fragment_shader == other.fragment_shader
			   && primitive_type == other.primitive_type
			   && cull_mode == other.cull_mode
			   && enable_depth_test == other.enable_depth_test
			   && enable_depth_write == other.enable_depth_write
			   && (!enable_depth_test || compare_op == other.compare_op);
	}
};

struct MaterialInstance {
	std::string name{};
	MaterialState state{};

	glm::vec4 base_color{1.0f, 1.0f, 1.0f, 1.0f};

	// TODO:
	// Handle albedo_texture{};
	// Handle normal_texture{};
	// Handle orm_texture{};
};

template <> struct std::hash<MaterialState> {
	size_t operator() (const MaterialState& material) const noexcept {
		size_t h = 0;

		hash_combine (h, std::hash<std::string>{}(material.vertex_shader));
		hash_combine (h, std::hash<std::string>{}(material.fragment_shader));

		hash_combine (
			h,
			std::hash<uint8_t>{}(static_cast<uint8_t> (material.primitive_type))
		);
		hash_combine (
			h, std::hash<uint8_t>{}(static_cast<uint8_t> (material.cull_mode))
		);

		hash_combine (h, std::hash<bool>{}(material.enable_depth_test));
		hash_combine (h, std::hash<bool>{}(material.enable_depth_write));

		if (material.enable_depth_test) {
			hash_combine (
				h,
				std::hash<uint8_t>{}(static_cast<uint8_t> (material.compare_op))
			);
		}

		return h;
	}
};

namespace Materials {

inline static MaterialInstance Geometry{
	.name = "geometry",
	.state
	= {.vertex_shader = "geometry",
	   .fragment_shader = "gbuffer",
	   .primitive_type = PrimitiveType::TriangleList,
	   .cull_mode = CullMode::Back,
	   .compare_op = CompareOp::Less,
	   .enable_depth_test = true,
	   .enable_depth_write = true},
	.base_color = {1.0f, 1.0f, 1.0f, 1.0f}
};

inline static MaterialInstance Deferred{
	.name = "deferred",
	.state
	= {.vertex_shader = "screen",
	   .fragment_shader = "lighting",
	   .primitive_type = PrimitiveType::TriangleList,
	   .cull_mode = CullMode::Back,
	   .compare_op = CompareOp::Less,
	   .enable_depth_test = false,
	   .enable_depth_write = false},
	.base_color = {1.0f, 1.0f, 1.0f, 1.0f}
};

}

#endif // MATERIAL_H
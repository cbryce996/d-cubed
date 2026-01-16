#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
	const std::string name;
	const std::string shader_name;

	const SDL_GPUPrimitiveType primitive_type;
	const SDL_GPUCullMode cull_mode;
	const SDL_GPUCompareOp compare_op;

	const bool enable_depth_test;
	const bool enable_depth_write;

	glm::vec4 base_color = {1.0f, 1.0f, 1.0f, 1.0f};
};

#endif // MATERIAL_H

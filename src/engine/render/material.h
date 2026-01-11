#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
	std::string name;
	std::string shader_name;
	std::string pipeline_name;

	glm::vec4 base_color = {1.0f, 1.0f, 1.0f, 1.0f};
};
#endif // MATERIAL_H

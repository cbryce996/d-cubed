#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
	std::string name;		   // e.g., "RustyMetal"
	std::string shader_name;   // e.g., "lit_pbr"
	std::string pipeline_name; // e.g., "opaque_backcull"

	// You can also store material-specific parameters here
	glm::vec4 base_color = {1.0f, 1.0f, 1.0f, 1.0f};
};
#endif // MATERIAL_H

#ifndef INSTANCE_H
#define INSTANCE_H

#include <glm/glm.hpp>

struct Instance {
	glm::vec3 basePos;
	glm::vec3 rotAxis;
	float phase;
	glm::vec3 scale;
	float pad;
};

#endif // INSTANCE_H

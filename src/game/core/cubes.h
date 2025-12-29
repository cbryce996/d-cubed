#ifndef CUBES_H
#define CUBES_H

#include <glm/glm.hpp>
#include <vector>

struct Cubes {
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

inline Cubes cubes;

#endif // CUBES_H

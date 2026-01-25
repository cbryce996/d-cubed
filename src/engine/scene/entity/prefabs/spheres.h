#ifndef SPHERES_H
#define SPHERES_H

#include "entity/entity.h"

#include <glm/glm.hpp>

class InstancingComponent;
struct Block;

class Spheres final : public IEntity {
  public:
	explicit Spheres (
		glm::vec3 offset = glm::vec3 (0.0f), float rotation_radians = 0.0f,
		float phase_offset = 0.0f, std::string name = "Wave"
	);

	void on_load () override;
	void on_unload () override;

	void update (float dt_ms, float sim_time_ms) override;

  private:
	void setup_grid (InstancingComponent& instancing) const;

	glm::vec3 offset;
	float rotation;
	float phase_offset;

	static constexpr int GRID_SIZE = 128;
	static constexpr float SPACING = 1.5f;
};

#endif // SPHERES_H

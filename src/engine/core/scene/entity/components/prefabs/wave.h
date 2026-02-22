#ifndef WAVE_H
#define WAVE_H

#include "entity/components/component.h"

#include <glm/glm.hpp>

class InstancingComponent;

class WaveComponent final : public IEntityComponent {
  public:
	WaveComponent (const glm::vec3 origin, const float phase_offset)
		: origin (origin), phase_offset (phase_offset) {}

	void apply (InstancingComponent& instancing_component, float time) const;

  private:
	glm::vec3 origin;
	float phase_offset;
};

#endif // WAVE_H

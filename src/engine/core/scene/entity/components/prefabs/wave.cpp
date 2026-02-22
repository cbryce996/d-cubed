#include "wave.h"

#include "instancing.h"

void WaveComponent::apply (
	InstancingComponent& instancing_component, const float time
) const {
	for (auto& transform : instancing_component.instances) {
		auto& pos = transform.position;
		const float dist = glm::length (
			glm::vec2 (pos.x - origin.x, pos.z - origin.z)
		);
		pos.y = sin (dist * 0.3f - time + phase_offset) * 2.0f;
	}
}
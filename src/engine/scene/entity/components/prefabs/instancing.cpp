#include "instancing.h"
#include "utils.h"

void InstancingComponent::pack (std::vector<Block>& out) const {
	for (const auto& [position, rotation, scale] : instances) {
		Block block{};
		write_vec4 (block, 0, glm::vec4 (position, 1.0f));
		write_vec4 (
			block, 1, glm::vec4 (rotation.x, rotation.y, rotation.z, rotation.w)
		);
		write_vec4 (block, 2, glm::vec4 (scale, 0.0f));
		write_vec4 (block, 3, glm::vec4 (0.0f));
		out.push_back (block);
	}
}
#include "instancing.h"
#include "utils.h"

void InstancingComponent::pack (
	const glm::mat4& base_world, std::vector<Block>& out
) const {
	out.reserve (out.size () + instances.size ());

	for (const auto& inst : instances) {
		const glm::mat4 local = inst.to_mat4 ();
		const glm::mat4 world = base_world * local;

		Block block{};
		write_vec4 (block, 0, world[0]);
		write_vec4 (block, 1, world[1]);
		write_vec4 (block, 2, world[2]);
		write_vec4 (block, 3, world[3]);
		out.push_back (block);
	}
}

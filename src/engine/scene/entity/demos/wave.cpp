#include "wave.h"

#include "maths/geometry/cube.h"
#include "render/material.h"
#include "render/render.h"

Wave::Wave (
	glm::vec3 offset_, float rotation_radians_, float phase_offset_,
	std::string name
)
	: IEntity (std::move (name)), offset (offset_),
	  rotation (rotation_radians_), phase_offset (phase_offset_) {}

void Wave::setup_grid () {
	instancing_component = std::make_unique<InstancingComponent> ();
	instancing_component->instances.clear ();
	instancing_component->instances.reserve (GRID_SIZE * GRID_SIZE);

	const float c = cos (rotation);
	const float s = sin (rotation);

	for (int z = 0; z < GRID_SIZE; ++z) {
		for (int x = 0; x < GRID_SIZE; ++x) {

			glm::vec3 local_pos (
				(x - GRID_SIZE / 2) * SPACING, 0.0f,
				(z - GRID_SIZE / 2) * SPACING
			);

			glm::vec3 rotated_pos (
				local_pos.x * c - local_pos.z * s, 0.0f,
				local_pos.x * s + local_pos.z * c
			);

			Transform t;
			t.position = rotated_pos; // local to entity
			t.scale = glm::vec3 (1.0f);

			instancing_component->instances.push_back (t);
		}
	}
}

void Wave::on_load () {
	static std::shared_ptr<MeshInstance> cube_mesh
		= std::make_shared<MeshInstance> (Cube::generate (0.5f));

	MeshTransfer::to_gpu (*cube_mesh);

	mesh = cube_mesh.get ();
	material = &Materials::Geometry;

	setup_grid ();
}

void Wave::on_unload () { instancing_component->instances.clear (); }

void Wave::update (float dt_ms, float sim_time_ms) {
	const float time = sim_time_ms * 0.001f + phase_offset;

	for (Transform& t : instancing_component->instances) {
		glm::vec3& pos = t.position;

		const float dist = glm::length (
			glm::vec2 (pos.x - offset.x, pos.z - offset.z)
		);

		pos.y = sin (dist * 0.3f - time) * 2.0f;
	}
}

inline void Wave::pack_instances (std::vector<Block>& out_blocks) const {
	for (auto& [position, rotation, scale] : instancing_component->instances) {
		Block block{};
		write_vec4 (block, 0, glm::vec4 (position, 1.0f));
		write_vec4 (
			block, 1, glm::vec4 (rotation.x, rotation.y, rotation.z, rotation.w)
		);
		write_vec4 (block, 2, glm::vec4 (scale, 0.0f));
		write_vec4 (block, 3, glm::vec4 (0.0f));
		out_blocks.push_back (block);
	}
}

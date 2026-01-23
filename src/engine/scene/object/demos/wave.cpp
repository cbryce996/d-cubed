#include "wave.h"

#include "maths/geometry/cube.h"
#include "render/material.h"
#include "render/render.h"

Wave::Wave (
	glm::vec3 offset_, float rotation_radians_, float phase_offset_,
	std::string name
)
	: ISceneObject (std::move (name)), offset (offset_),
	  rotation (rotation_radians_), phase_offset (phase_offset_) {}

void Wave::setup_grid () {
	instances.clear ();
	instances.reserve (GRID_SIZE * GRID_SIZE);

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

			glm::vec3 world_pos = rotated_pos + offset;

			Block block{};
			block.clear ();
			block.write (0, glm::vec4 (world_pos, 0.0f));
			block.write (1, glm::vec4 (0, 1, 0, 0));
			block.write (2, glm::vec4 (1.0f));

			instances.push_back (block);
		}
	}
}

void Wave::on_load () { setup_grid (); }

void Wave::on_unload () { instances.clear (); }

void Wave::collect_drawables (RenderState& out_render_state) {
	static std::shared_ptr<Mesh> cube_mesh = std::make_shared<Mesh> (
		Cube::generate (0.5f)
	);

	cube_mesh->to_gpu ();

	batch.blocks = instances;

	out_render_state.drawables.push_back (
		{.mesh = cube_mesh.get (),
		 .material = &Materials::Geometry,
		 .instance_batch = &batch}
	);
}

void Wave::update (float dt_ms, const float sim_time_ms) {
	const float time = sim_time_ms * 0.001f + phase_offset;

	for (auto& block : instances) {
		glm::vec4 pos = block.to_cpu (0);

		float dist = glm::length (
			glm::vec2 (pos.x - offset.x, pos.z - offset.z)
		);
		pos.y = sin (dist * 0.3f - time) * 2.0f;

		block.write (0, pos);
	}
}

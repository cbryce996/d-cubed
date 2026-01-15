#include "wave.h"

#include "maths/geometry/cube.h"
#include "render/material.h"

void WaveDemo::setup_grid () {
	instances.clear ();
	instances.reserve (GRID_SIZE * GRID_SIZE);

	for (int z = 0; z < GRID_SIZE; ++z) {
		for (int x = 0; x < GRID_SIZE; ++x) {
			const float fx = (x - GRID_SIZE / 2) * SPACING;
			const float fz = (z - GRID_SIZE / 2) * SPACING;

			glm::vec3 position (fx, 0.0f, fz);
			glm::vec3 axis (0.0f, 1.0f, 0.0f);

			Block block{};
			block.clear ();
			block.write (0, glm::vec4 (position, 0.0f));
			block.write (1, glm::vec4 (axis, 0.0f));
			block.write (2, glm::vec4 (1.0f));

			instances.push_back (block);
		}
	}
}

void WaveDemo::on_load () { setup_grid (); }

void WaveDemo::on_unload () { instances.clear (); }

void WaveDemo::build_render_state (RenderState& render_state) {
	static std::shared_ptr<Mesh> cube_mesh = std::make_shared<Mesh> (
		Cube::generate (1.0f)
	);

	cube_mesh->to_gpu ();

	static InstanceBatch batch;

	static Material material{
		.name = "grid_material",
		.pipeline_name = "lit_opaque_backcull",
		.shader_name = "anomaly"
	};

	batch.blocks = instances;

	render_state.drawables.push_back (
		{.mesh = cube_mesh.get (),
		 .material = &material,
		 .instance_batch = &batch}
	);
}

void WaveDemo::fixed_update (float dt_ms, const float sim_time_ms) {
	const float time = sim_time_ms * 0.005f;

	for (auto& block : instances) {
		glm::vec4 pos = block.to_cpu (0);

		const float dist = glm::length (glm::vec2 (pos.x, pos.z));

		pos.y = sin (dist * 0.1f - time) * 2.0f;
		block.write (0, pos);
	}
}

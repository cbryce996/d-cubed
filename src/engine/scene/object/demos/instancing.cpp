#include "instancing.h"

#include "maths/geometry/cube.h"
#include "maths/geometry/sphere.h"
#include "render/material.h"

#include <glm/glm.hpp>
#include <random>

InstancingDemo::InstancingDemo () : ISceneObject ("InstancingDemo") {}

void InstancingDemo::setup_instances (
	std::vector<Block>& out, int count, uint32_t seed
) {
	std::mt19937 rng (seed);

	std::uniform_real_distribution<float> pos (-50.f, 50.f);
	std::uniform_real_distribution<float> scale_dist (0.5f, 2.0f);
	std::uniform_real_distribution<float> axis_dist (-1.f, 1.f);
	std::uniform_real_distribution<float> angle_dist (
		0.f, glm::two_pi<float> ()
	);

	out.clear ();
	out.reserve (count);

	for (int i = 0; i < count; ++i) {
		glm::vec3 position (pos (rng), pos (rng), pos (rng));

		glm::vec3 axis (axis_dist (rng), axis_dist (rng), axis_dist (rng));
		if (glm::length (axis) < 0.001f)
			axis = glm::vec3 (0, 1, 0);
		axis = glm::normalize (axis);

		float angle = angle_dist (rng);
		float scale = scale_dist (rng);

		Block b{};
		b.clear ();
		b.write (0, glm::vec4 (position, angle));
		b.write (1, glm::vec4 (axis, 0.0f));
		b.write (2, glm::vec4 (scale));

		out.push_back (b);
	}
}

void InstancingDemo::on_load () {
	setup_instances (sphere_instances, NUM_SPHERES, 42);
	setup_instances (cube_instances, NUM_CUBES, 1337);
}

void InstancingDemo::on_unload () {
	sphere_instances.clear ();
	cube_instances.clear ();
}

void InstancingDemo::update (float dt_ms, const float sim_time_ms) {
	const float t = sim_time_ms * 0.001f;

	auto animate = [&] (std::vector<Block>& instances) {
		for (auto& block : instances) {
			glm::vec4 pos_angle = block.to_cpu (0);

			pos_angle.w = t;

			block.write (0, pos_angle);
		}
	};

	animate (sphere_instances);
	animate (cube_instances);
}

void InstancingDemo::collect_drawables (RenderState& out_render_state) {
	static auto sphere_mesh = std::make_shared<Mesh> (
		Sphere::generate (1.0f, 20, 20)
	);
	static auto cube_mesh = std::make_shared<Mesh> (Cube::generate (1.0f));

	sphere_mesh->to_gpu ();
	cube_mesh->to_gpu ();

	static InstanceBatch sphere_batch;
	static InstanceBatch cube_batch;

	static Material material{
		.name = "default_instanced",
		.pipeline_name = "lit_opaque_backcull",
		.shader_name = "geometry_basic"
	};

	sphere_batch.blocks = sphere_instances;
	cube_batch.blocks = cube_instances;

	out_render_state.drawables.push_back (
		{.mesh = sphere_mesh.get (),
		 .material = &material,
		 .instance_batch = &sphere_batch}
	);

	out_render_state.drawables.push_back (
		{.mesh = cube_mesh.get (),
		 .material = &material,
		 .instance_batch = &cube_batch}
	);
}

#include "spheres.h"
#include "entity/components/prefabs/instancing.h"
#include "entity/components/prefabs/wave.h"
#include "maths/geometry/sphere.h"
#include "render/material.h"

Spheres::Spheres (
	glm::vec3 offset_, float rotation_radians_, float phase_offset_,
	std::string name, MeshInstance* mesh, MaterialInstance* material,
	const Transform& transform, const Transform& world_transform
)
	: IEntity (std::move (name), mesh, material, transform, world_transform),
	  offset (offset_), rotation (rotation_radians_),
	  phase_offset (phase_offset_) {}

void Spheres::setup_grid (InstancingComponent& instancing) const {
	instancing.instances.clear ();
	instancing.instances.reserve (GRID_SIZE * GRID_SIZE);

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
			t.position = rotated_pos;
			t.scale = glm::vec3 (1.0f);

			instancing.instances.push_back (t);
		}
	}
}

void Spheres::on_load () {
	static std::shared_ptr<MeshInstance> mesh_inst
		= std::make_shared<MeshInstance> (Sphere::generate (0.2f, 10, 10));

	MeshTransfer::to_gpu (*mesh_inst);

	mesh = mesh_inst.get ();
	material = &Materials::Geometry;

	// Components
	auto& instancing = add_component<InstancingComponent> ();
	auto& wave = add_component<WaveComponent> (offset, phase_offset);

	setup_grid (instancing);
}

void Spheres::on_unload () {}

void Spheres::update (float dt_ms, float sim_time_ms) {
	auto* inst = get_component<InstancingComponent> ();
	auto* wave = get_component<WaveComponent> ();

	if (!inst || !wave)
		return;

	float time = sim_time_ms * 0.001f;
	wave->apply (*inst, time);
}

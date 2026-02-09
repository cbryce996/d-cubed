#include "scene.h"

#include "cameras/camera.h"
#include "entity/components/prefabs/instancing.h"
#include "entity/entity.h"
#include "render/drawable.h"
#include "render/render.h"

#include <ranges>

Scene::Scene () {
	Camera camera{};
	camera.name = "main";
	camera.transform.position = glm::vec3 (0.0f, -20.0f, 20.0f);
	camera.transform.rotation = glm::quat (
		glm::vec3 (glm::radians (30.0f), 0.0f, 0.0f)
	);
	camera.transform.scale = glm::vec3 (1.0f);
	camera.lens.fov = 100.0f;
	camera.lens.aspect = 16.0f / 9.0f;
	camera.lens.near_clip = 0.1f;
	camera.lens.far_clip = 300.0f;
	camera.move_speed = 0.1f;
	camera.look_sensitivity = 0.1f;

	camera_manager = std::make_unique<CameraManager> (camera);
}

void Scene::on_load () {
	for (const auto& entity : scene_entities | std::views::values) {
		entity->on_load ();
	}
	loaded = true;
}

void Scene::on_unload () {
	for (const auto& entity : scene_entities | std::views::values) {
		entity->on_unload ();
	}
	loaded = false;
}

void Scene::update (const float dt_ms, const float sim_time_ms) {
	for (const auto& entity : scene_entities | std::views::values) {
		entity->update_world_transform ();
		entity->update (dt_ms, sim_time_ms);
	}
}

void Scene::collect_drawables (RenderState& out_render_state) {
	for (const auto& entity : scene_entities | std::views::values) {
		Drawable drawable = {
			.mesh = entity->mesh,
			.material = entity->material,
			.transform = entity->transform
		};

		drawable.instance_blocks.clear ();

		if (entity->has_component<InstancingComponent> ()) {
			const auto* inst = entity->get_component<InstancingComponent> ();
			inst->pack (drawable.instance_blocks);
		}

		out_render_state.drawables.push_back (drawable);
	}
}

void Scene::add_entity (std::unique_ptr<IEntity> entity) {
	assert (entity);
	assert (!entity->name.empty ());

	std::string name = entity->name;

	if (scene_entities.contains (name)) {
		return;
	}

	if (loaded) {
		entity->on_load ();
	}

	scene_entities.emplace (name, std::move (entity));
}

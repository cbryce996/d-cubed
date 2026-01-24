#include "scene.h"

#include "entity/entity.h"
#include "render/drawable.h"
#include "render/render.h"

#include <ranges>

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
		entity->pack_instances (drawable.instance_blocks);

		out_render_state.drawables.push_back (drawable);
	}
}

void Scene::add_entity (std::unique_ptr<IEntity> entity) {
	const std::string& name = entity->name;

	if (scene_entities.contains (name)) {
		// Log error
		return;
	}

	// If the scene is already active, load the object immediately
	if (loaded) {
		entity->on_load ();
	}

	scene_entities.emplace (name, std::move (entity));
}

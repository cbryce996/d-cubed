#include "scene.h"

#include "object/object.h"

#include <ranges>

void Scene::on_load () {
	for (const auto& object : scene_objects | std::views::values) {
		object->on_load ();
	}
	loaded = true;
}

void Scene::on_unload () {
	for (const auto& object : scene_objects | std::views::values) {
		object->on_unload ();
	}
	loaded = false;
}

void Scene::update (const float dt_ms, const float sim_time_ms) {
	for (const auto& object : scene_objects | std::views::values) {
		object->update (dt_ms, sim_time_ms);
	}
}

void Scene::collect_drawables (RenderState& out_render_state) {
	for (const auto& object : scene_objects | std::views::values) {
		object->collect_drawables (out_render_state);
	}
}

void Scene::add_object (std::unique_ptr<ISceneObject> object) {
	const std::string& name = object->name;

	if (scene_objects.contains (name)) {
		// Log error
		return;
	}

	// If the scene is already active, load the object immediately
	if (loaded) {
		object->on_load ();
	}

	scene_objects.emplace (name, std::move (object));
}

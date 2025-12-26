#include "game.h"

#include <iostream>

#include "crafting.h"
#include "engine/engine.h"
#include "items.h"
#include "recipes.h"
#include "render/render.h"
#include "types.h"
#include "update.h"
#include "utils.h"

inline float fast_exp(const float x) {
	// Approximate exp(x) using a 5th-degree
	// polynomial Accurate for x in [-6, 0]
	return 1.0f + x * (1.0f + x * (0.5f + x * (1.0f / 6.0f + x * (1.0f / 24.0f + x * (1.0f / 120.0f)))));
}

GameManager::GameManager(
	std::shared_ptr<AssetManager> asset_manager,
	std::shared_ptr<ShaderManager> shader_manager
)
	: asset_manager(std::move(asset_manager)),
	  shader_manager(std::move(shader_manager)) {
	update_managers.emplace_back(2500.0f, [this](const float delta_time_ms) { calculate_item_decays(delta_time_ms); });

	update_managers.emplace_back(1000.0f, [this](const float delta_time_ms) {
		calculate_item_crafting_progress(delta_time_ms);
	});

	initialize_item_types();
}

GameManager::~GameManager() = default;

void GameManager::update(
	const float delta_time,
	TaskScheduler& scheduler,
	const InputManager& input
) {
	delta_time_ms = delta_time;

	for (auto& manager : update_managers) {
		manager.update(delta_time, scheduler);
	}

	player.update(input, delta_time);
}

void GameManager::handle_input(const InputManager& input) {
	// TODO
}

void GameManager::calculate_item_decays(const float delta_time_ms) {
	std::cout << "[Items] Calculating item decays\n";

	const size_t count = item_decays.ages.size();
	for (size_t i = 0; i < count; ++i) {
		// Update age in milliseconds
		item_decays.ages[i] += delta_time_ms;

		const float rate = item_type_decays.decay_rates[items.type_ids[i]];
		const float age = item_decays.ages[i];

		const float decay = fast_exp(-rate * (age / 1000.0f));	// Convert to seconds if needed
		item_decays.decays[i] = 1.0f - decay;

		std::cout << "[Items] Item " << i << " age: " << age << " | decay: " << item_decays.decays[i] << "\n";
	}
}

void GameManager::calculate_item_crafting_progress(const float delta_time_ms) {
	std::cout << "[Recipes] Updating crafting progress\n";

	const size_t count = item_crafting.ids.size();

	for (size_t i = 0; i < count; ++i) {
		const size_t recipe_id = item_crafting.ids[i];
		const float crafting_time = item_recipes.crafting_times[recipe_id];

		item_crafting.progress[i] += delta_time_ms;

		if (item_crafting.progress[i] >= crafting_time) {
			const size_t output_type = item_recipe_outputs.outputs[recipe_id];
			const float output_amount = item_recipe_outputs.output_amounts[recipe_id];

			std::cout << "Recipe " << recipe_id << " completed. Produced item type: " << output_type << " (x"
					  << output_amount << ")\n";

			// Reset or remove the recipe from crafting
			item_crafting.progress[i] = 0.0f;

			// Future: actually push to some item inventory or trigger event
		}
	}
}

void GameManager::write_render_state(const float elapsed_time) {
	render_state.drawables.clear();

	static std::shared_ptr<Mesh> cube_mesh = create_cube_mesh();

	static Material material;
	material.pipeline_config = {
		.name = "cube_pipeline",
		.shader = shader_manager->get_shader("lit"),
	};

	for (int i = 0; i < 10; ++i) {
		Entity cube;
		cube.mesh = cube_mesh.get();  // safe, persists
		cube.material = &material;

		// Animate positions
		float slow_factor = 0.001f;	 // 10x slower
		cube.transform.position = glm::vec3(
			i * 1.5f,
			std::sin(elapsed_time * 2.0f * slow_factor + i) * 0.5f,
			std::cos(elapsed_time * 1.5f * slow_factor + i) * 0.5f
		);
		cube.transform.rotation = glm::angleAxis((elapsed_time + i) * slow_factor, glm::vec3(0, 1, 0));

		Drawable drawable{};
		drawable.mesh = cube.mesh;
		drawable.material = cube.material;
		drawable.model = compute_model_matrix(cube.transform);

		render_state.drawables.push_back(drawable);
	}
}
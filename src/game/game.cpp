#include "game.h"

#include <iostream>

#include "crafting.h"
#include "items.h"
#include "recipes.h"
#include "types.h"
#include "engine/update.h"
#include "render/render.h"

inline float fast_exp(const float x) {
	// Approximate exp(x) using a 5th-degree
	// polynomial Accurate for x in [-6, 0]
	return 1.0f + x * (1.0f + x * (0.5f + x * (1.0f / 6.0f + x * (1.0f / 24.0f + x * (1.0f / 120.0f)))));
}

GameManager::GameManager() {
	update_managers.emplace_back(
		3000.0f,
		[this]() { calculate_item_decays(); }
	);

	update_managers.emplace_back(
		1000.0f,
		[this]() { calculate_item_crafting_progress(); }
	);
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

void GameManager::calculate_item_decays() {
	std::cout << "[Items] Calculating item decays"
			  << "\n";
	const size_t count = item_decays.ages.size();
	for (size_t i = 0; i < count; ++i) {
		const float rate = item_type_decays.decay_rates[items.type_ids[i]];
		const float age = item_decays.ages[i];
		item_decays.decays[i] = 1.0f - fast_exp(-rate * age);
	}
}

void GameManager::calculate_item_crafting_progress() const {
	std::cout << "[Recipes] Updating crafting progress\n";

	const size_t count = item_crafting.ids.size();

	for (size_t i = 0; i < count; ++i) {
		const size_t recipe_id = item_crafting.ids[i];
		const float crafting_time = item_recipes.crafting_times[recipe_id];

		item_crafting.progress[i] += delta_time_ms;

		if (item_crafting.progress[i] >= crafting_time) {
			const size_t output_type = item_recipe_outputs.outputs[recipe_id];
			const float output_amount = item_recipe_outputs.output_amounts[recipe_id];

			std::cout << "Recipe " << recipe_id << " completed. Produced item type: "
					  << output_type << " (x" << output_amount << ")\n";

			// Reset or remove the recipe from crafting
			item_crafting.progress[i] = 0.0f;

			// Future: actually push to some item inventory or trigger event
		}
	}
}

void GameManager::write_render_state(RenderState& render_state) const {
	render_state.item_rects.clear();
	const size_t item_count = items.ids.size();

	for (size_t i = 0; i < item_count; ++i) {
		SDL_Rect rect;
		rect.x = std::rand() % 800;
		rect.y = std::rand() % 600;
		rect.w = 10;
		rect.h = 10;
		render_state.item_rects.push_back(rect);
	}

	render_state.player_rect = player.get_rect();
}
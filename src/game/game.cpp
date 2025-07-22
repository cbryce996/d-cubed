#include "game.h"

#include <iostream>

#include "crafting.h"
#include "items.h"
#include "recipes.h"
#include "types.h"
#include "update.h"
#include "engine/engine.h"
#include "render/render.h"

inline float fast_exp(const float x) {
	// Approximate exp(x) using a 5th-degree
	// polynomial Accurate for x in [-6, 0]
	return 1.0f + x * (1.0f + x * (0.5f + x * (1.0f / 6.0f + x * (1.0f / 24.0f + x * (1.0f / 120.0f)))));
}

GameManager::GameManager() {
	update_managers.emplace_back(2500.0f, [this](const float delta_time_ms) {
		calculate_item_decays(delta_time_ms);
	});

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
	if (input.is_mouse_pressed(SDL_BUTTON_LEFT)) {
		std::cout << "[Input] Mouse pressed" << "\n";
		const SDL_Point pos = input.get_mouse_position();

		register_item(
			1,
			1,
			5,
			0,
			0,
			pos.x,
			pos.y
		);
		std::cout << "[Input] Spawned item at " << pos.x << "," << pos.y << "\n";
	}
}


void GameManager::calculate_item_decays(const float delta_time_ms) {
	std::cout << "[Items] Calculating item decays\n";

	const size_t count = item_decays.ages.size();
	for (size_t i = 0; i < count; ++i) {
		// Update age in milliseconds
		item_decays.ages[i] += delta_time_ms;

		const float rate = item_type_decays.decay_rates[items.type_ids[i]];
		const float age = item_decays.ages[i];

		const float decay = fast_exp(-rate * (age / 1000.0f)); // Convert to seconds if needed
		item_decays.decays[i] = 1.0f - decay;

		std::cout << "[Items] Item " << i << " age: " << age
				  << " | decay: " << item_decays.decays[i] << "\n";
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
		rect.x = items.x[i];
		rect.y = items.y[i];
		rect.w = 10;
		rect.h = 10;
		render_state.item_rects.push_back(rect);
	}

	render_state.player_rect = player.get_rect();
}
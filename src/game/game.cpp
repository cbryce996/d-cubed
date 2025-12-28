#include "game.h"

#include "clock.h"

#include <iostream>

#include "../engine/tasks/schedule.h"
#include "engine/render/material.h"
#include "engine/utils.h"
#include "game/core/items/crafting.h"
#include "game/core/items/items.h"
#include "game/core/items/recipes.h"
#include "game/core/items/types.h"
#include "render/render.h"

inline float fast_exp (const float x) {
	// Approximate exp(x) using a 5th-degree
	// polynomial Accurate for x in [-6, 0]
	return 1.0f
		   + x
				 * (1.0f
					+ x
						  * (0.5f
							 + x
								   * (1.0f / 6.0f
									  + x
											* (1.0f / 24.0f
											   + x * (1.0f / 120.0f)))));
}

Game::Game (std::unique_ptr<Engine> engine) : engine (std::move (engine)) {
	this->engine->simulation->schedules.emplace_back (
		2500.0f, [this] (const float delta_time_ms) {
			calculate_item_decays (delta_time_ms);
		}
	);

	this->engine->simulation->schedules.emplace_back (
		1000.0f, [this] (const float delta_time_ms) {
			calculate_item_crafting_progress (delta_time_ms);
		}
	);

	initialize_item_types ();
}

Game::~Game () = default;

void Game::run () {
	running = true;

	engine->simulation->task_scheduler.start ();

	Clock clock (60);

	while (running) {
		// --- FRAME BEGIN ---
		float dt = clock.begin_frame ();

		// --- EVENTS ---
		SDL_Event e;
		while (SDL_PollEvent (&e)) {
			if (e.type == SDL_EVENT_QUIT)
				running = false;
		}

		// --- INPUT ---
		engine->input.poll ();
		handle_input (engine->input);

		const KeyboardInput& keyboard = engine->input.get_keyboard_input ();
		const MouseInput& mouse = engine->input.get_mouse_input ();

		if (keyboard.keys[SDL_SCANCODE_ESCAPE])
			running = false;

		// --- FIXED SIMULATION ---
		while (clock.should_step_simulation ()) {
			engine->simulation->update (clock.fixed_dt_ms);
			clock.consume_simulation_step ();
		}

		// --- RENDER ---
		float alpha = clock.alpha ();

		RenderState render_state{};
		write_game_state (
			engine->simulation->simulation_time_ms + alpha * clock.fixed_dt_ms,
			&render_state
		);

		engine->render->camera_manager->update_camera_position (
			dt, keyboard.keys
		);
		engine->render->camera_manager->update_camera_look (&mouse);

		engine->render->render (&render_state);

		// --- FRAME END ---
		clock.end_frame ();

		if (clock.should_log_stats ()) {
			std::cout << "[Perf] FPS: " << clock.avg_fps << "\n";
		}
	}

	engine->simulation->task_scheduler.stop ();
}

void Game::handle_input (const InputManager& input) {
	// TODO
}

void Game::write_game_state (
	const float elapsed_time, RenderState* render_state
) const {
	render_state->drawables.clear ();

	static std::shared_ptr<Mesh> cube_mesh = create_cube_mesh ();

	static Material material;
	material.pipeline_config = {
		.name = "cube_pipeline",
		.shader = engine->render->shader_manager->get_shader ("lit"),
	};

	for (int i = 0; i < 10; ++i) {
		Entity cube;
		cube.mesh = cube_mesh.get ();
		cube.material = &material;

		float slow_factor = 0.001f;
		cube.transform.position = glm::vec3 (
			i * 1.5f, std::sin (elapsed_time * 2.0f * slow_factor + i) * 0.5f,
			std::cos (elapsed_time * 1.5f * slow_factor + i) * 0.5f
		);
		cube.transform.rotation = glm::angleAxis (
			(elapsed_time + i) * slow_factor, glm::vec3 (0, 1, 0)
		);

		Drawable drawable{};
		drawable.mesh = cube.mesh;
		drawable.material = cube.material;
		drawable.model = compute_model_matrix (cube.transform);

		render_state->drawables.push_back (drawable);
	}
}

void Game::calculate_item_decays (const float delta_time_ms) {
	std::cout << "[Items] Calculating item decays\n";

	const size_t count = item_decays.ages.size ();
	for (size_t i = 0; i < count; ++i) {
		// Update age in milliseconds
		item_decays.ages[i] += delta_time_ms;

		const float rate = item_type_decays.decay_rates[items.type_ids[i]];
		const float age = item_decays.ages[i];

		const float decay = fast_exp (
			-rate * (age / 1000.0f)
		); // Convert to seconds if needed
		item_decays.decays[i] = 1.0f - decay;

		std::cout << "[Items] Item " << i << " age: " << age
				  << " | decay: " << item_decays.decays[i] << "\n";
	}
}

void Game::calculate_item_crafting_progress (const float delta_time_ms) {
	std::cout << "[Recipes] Updating crafting progress\n";

	const size_t count = item_crafting.ids.size ();

	for (size_t i = 0; i < count; ++i) {
		const size_t recipe_id = item_crafting.ids[i];
		const float crafting_time = item_recipes.crafting_times[recipe_id];

		item_crafting.progress[i] += delta_time_ms;

		if (item_crafting.progress[i] >= crafting_time) {
			const size_t output_type = item_recipe_outputs.outputs[recipe_id];
			const float output_amount
				= item_recipe_outputs.output_amounts[recipe_id];

			std::cout << "Recipe " << recipe_id
					  << " completed. Produced item type: " << output_type
					  << " (x" << output_amount << ")\n";

			item_crafting.progress[i] = 0.0f;
		}
	}
}
#include "game.h"

#include "clock.h"

#include <iostream>

#include "core/cubes.h"
#include "engine/render/material.h"
#include "engine/utils.h"
#include "game/core/items/crafting.h"
#include "game/core/items/items.h"
#include "game/core/items/recipes.h"
#include "game/core/items/types.h"
#include "render/render.h"

#include <random>

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

	setup_cubes ();

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
			if (e.type == SDL_EVENT_WINDOW_RESIZED) {
				engine->render->resize (e.window.data1, e.window.data2);
			}
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
		write_game_state (&render_state);

		engine->render->camera_manager->update_camera_position (
			dt, keyboard.keys
		);
		engine->render->camera_manager->update_camera_look (&mouse);

		engine->render->render (
			&render_state,
			engine->simulation->simulation_time_ms + alpha * clock.fixed_dt_ms
		);

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

std::vector<glm::vec3> base_positions;
std::vector<glm::vec3> random_rot_axes;
static constexpr int NUM_CUBES = 1000000;
std::vector<Block> instances;

void Game::setup_cubes () {
	instances.resize (NUM_CUBES);

	// Random generators
	std::mt19937 rng (42);
	std::uniform_real_distribution<float> pos_dist (-50.0f, 50.0f);
	std::uniform_real_distribution<float> scale_dist (0.5f, 2.0f);
	std::uniform_real_distribution<float> rot_dist (-1.0f, 1.0f);
	std::uniform_real_distribution<float> phase_dist (0.0f, 6.28f);

	for (int i = 0; i < NUM_CUBES; ++i) {
		Collection builder;

		// 1. Generate random transform data for this specific instance
		glm::vec3 random_pos = glm::vec3 (
			pos_dist (rng), pos_dist (rng), pos_dist (rng)
		);
		glm::vec3 random_axis = glm::normalize (
			glm::vec3 (rot_dist (rng), rot_dist (rng), rot_dist (rng))
		);
		float random_scale = scale_dist (rng);
		float random_phase = phase_dist (rng);

		builder.push(glm::vec4(random_pos, random_phase)); // Block 0
		builder.push(glm::vec4(random_axis, 0.0f));       // Block 1
		builder.push(glm::vec4(glm::vec3(random_scale), 1.0f)); // Block 2
		builder.push(glm::vec4(0.0f));                    // Block 3

		// Store ONLY the 64-byte data into the vector
		instances[i] = builder.storage;
	}
}

void Game::write_game_state (RenderState* render_state) {
	static std::shared_ptr<Mesh> cube_mesh = create_cube_mesh ();

	static Material material = {
		.name = "test_material",
		.pipeline_name = "lit_opaque_backcull",
		.shader_name = "anomaly"
	};

	Drawable cube{};
	cube.mesh = cube_mesh.get ();
	cube.material = &material;
	cube.instances.resize (NUM_CUBES);
	cube.instances_count = NUM_CUBES;
	cube.instances_size = sizeof (Block) * NUM_CUBES;

	cube.instances.resize(NUM_CUBES);

	std::memcpy(
	   cube.instances.data(),
	   instances.data(),
	   cube.instances_size
	);

	render_state->drawables.push_back(std::move(cube));
}

void Game::calculate_item_decays (const float delta_time_ms) {
	const size_t count = item_decays.ages.size ();
	for (size_t i = 0; i < count; ++i) {
		item_decays.ages[i] += delta_time_ms;

		const float rate = item_type_decays.decay_rates[items.type_ids[i]];
		const float age = item_decays.ages[i];

		const float decay = fast_exp (-rate * (age / 1000.0f));
		item_decays.decays[i] = 1.0f - decay;
	}
}

void Game::calculate_item_crafting_progress (const float delta_time_ms) {
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
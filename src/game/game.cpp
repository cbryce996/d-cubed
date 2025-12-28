#include "game.h"

#include <iostream>

#include "engine/render/material.h"
#include "engine/utils.h"
#include "game/core/items/crafting.h"
#include "game/core/items/items.h"
#include "game/core/items/recipes.h"
#include "game/core/items/types.h"
#include "render/render.h"
#include "update.h"

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

GameManager::GameManager (std::unique_ptr<Engine> engine)
	: engine (std::move (engine)) {
	update_managers.emplace_back (2500.0f, [this] (const float delta_time_ms) {
		calculate_item_decays (delta_time_ms);
	});

	update_managers.emplace_back (1000.0f, [this] (const float delta_time_ms) {
		calculate_item_crafting_progress (delta_time_ms);
	});

	initialize_item_types ();
}

GameManager::~GameManager () = default;

void GameManager::run () {
	using clock = std::chrono::high_resolution_clock;

	static_assert (sizeof (glm::vec3) == 12, "vec3 isn't 12 bytes?");
	static_assert (sizeof (Vertex) == 36, "Vertex struct is misaligned!");

	running = true;

	engine->task_scheduler.start ();

	constexpr int TARGET_FPS = 60;
	constexpr int FRAME_DELAY = 1000 / TARGET_FPS;

	int frame_counter = 0;
	float accumulated_frame_time_ms = 0.0f;
	auto last_stat_time = clock::now ();
	auto last_sim_time = clock::now ();

	Camera camera{};
	camera.name = "main";

	camera.transform.position = glm::vec3 (0.0f, 0.0f, 3.0f);
	camera.transform.rotation = glm::quat (
		glm::vec3 (0.0f, glm::radians (180.0f), 0.0f)
	);
	camera.transform.scale = glm::vec3 (1.0f);

	camera.lens.fov = 90.0f;
	camera.lens.aspect = 16.0f / 9.0f;
	camera.lens.near_clip = 0.1f;
	camera.lens.far_clip = 100.0f;

	camera.move_speed = 0.2f;
	camera.look_sensitivity = 0.1f;

	engine->render->camera_manager->add_camera (camera);
	engine->render->camera_manager->set_active_camera (camera);

	while (running) {
		auto frame_start = clock::now ();

		SDL_Event e;
		while (SDL_PollEvent (&e))
			if (e.type == SDL_EVENT_QUIT)
				running = false;

		auto current_time = clock::now ();
		const float delta_time_ms = std::chrono::duration<float, std::milli> (
										current_time - last_sim_time
		)
										.count ();
		last_sim_time = current_time;
		accumulated_frame_time_ms += delta_time_ms;

		engine->input.poll ();
		handle_input (engine->input);
		const KeyboardInput* keyboard_input
			= &engine->input.get_keyboard_input ();
		const MouseInput* mouse_input = &engine->input.get_mouse_input ();

		if (keyboard_input->keys[SDL_SCANCODE_ESCAPE])
			running = false;

		update (delta_time_ms, engine->task_scheduler, engine->input);
		RenderState* render_state = new RenderState();
		write_render_state (accumulated_frame_time_ms, render_state);

		engine->render->camera_manager->update_camera_position (
			delta_time_ms, keyboard_input->keys
		);
		engine->render->camera_manager->update_camera_look (mouse_input);
		engine->render->render (render_state);

		auto frame_end = clock::now ();
		float frame_time_ms = std::chrono::duration<float, std::milli> (
								  frame_end - frame_start
		)
								  .count ();

		if (frame_time_ms < FRAME_DELAY) {
			SDL_Delay (static_cast<Uint32> (FRAME_DELAY - frame_time_ms));

			auto delayed_end = clock::now ();
			frame_time_ms = std::chrono::duration<float, std::milli> (
								delayed_end - frame_start
			)
								.count ();
		}

		accumulated_frame_time_ms += frame_time_ms;
		frame_counter++;

		if (std::chrono::duration_cast<std::chrono::seconds> (
				frame_start - last_stat_time
			)
				.count ()
			>= 1) {
			float avg_frame_time = accumulated_frame_time_ms / frame_counter;
			float avg_fps = 1000.0f / avg_frame_time;

			std::cout << "[Perf] Avg Frame Time: " << avg_frame_time
					  << " ms | Avg FPS: " << avg_fps << "\n";

			accumulated_frame_time_ms = 0.0f;
			frame_counter = 0;
			last_stat_time = frame_start;
		}
	}

	engine->task_scheduler.stop ();
}

void GameManager::update (
	const float delta_time, TaskScheduler& scheduler, const InputManager& input
) {
	delta_time_ms = delta_time;

	for (auto& manager : update_managers)
		manager.update (delta_time, scheduler);

	player.update (input, delta_time);
}

void GameManager::handle_input (const InputManager& input) {
	// TODO
}

void GameManager::calculate_item_decays (const float delta_time_ms) {
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

void GameManager::calculate_item_crafting_progress (const float delta_time_ms) {
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

			// Reset or remove the recipe from crafting
			item_crafting.progress[i] = 0.0f;

			// Future: actually push to some item inventory or trigger
			// event
		}
	}
}

void GameManager::write_render_state (
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
		cube.mesh = cube_mesh.get (); // safe, persists
		cube.material = &material;

		// Animate positions
		float slow_factor = 0.001f; // 10x slower
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

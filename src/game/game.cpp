#include "game.h"
#include "clock.h"
#include "engine/render/material.h"
#include "engine/utils.h"
#include "maths/geometry/sphere.h"
#include "render/render.h"

#include <random>

Game::Game (std::unique_ptr<Engine> engine) : engine (std::move (engine)) {
	this->engine->simulation->schedules.emplace_back (
		2500.0f, [this] (const float delta_time_ms) { return; }
	);
}

Game::~Game () = default;

std::vector<glm::vec3> base_positions;
std::vector<glm::vec3> random_rot_axes;
static constexpr int NUM_INSTANCES = 1000;
std::vector<Block> instances;

void setup_spheres () {
	instances.resize (NUM_INSTANCES);

	std::mt19937 rng (42);
	std::uniform_real_distribution<float> pos_dist (-50.0f, 50.0f);
	std::uniform_real_distribution<float> scale_dist (0.5f, 2.0f);
	std::uniform_real_distribution<float> rot_dist (-1.0f, 1.0f);
	std::uniform_real_distribution<float> phase_dist (0.0f, 6.28f);

	for (int i = 0; i < NUM_INSTANCES; ++i) {
		glm::vec3 random_pos = glm::vec3 (
			pos_dist (rng), pos_dist (rng), pos_dist (rng)
		);

		glm::vec3 random_axis (rot_dist (rng), rot_dist (rng), rot_dist (rng));

		float len = glm::length (random_axis);
		if (len < 0.0001f) {
			random_axis = glm::vec3 (0.0f, 1.0f, 0.0f);
		} else {
			random_axis /= len;
		}

		float random_scale = scale_dist (rng);
		float random_phase = phase_dist (rng);

		Block block{};
		block.clear ();
		block.write (0, glm::vec4 (random_pos, random_phase));
		block.write (1, glm::vec4 (random_axis, random_phase));
		block.write (2, glm::vec4 (glm::vec3 (random_scale), 1.0f));

		instances[i] = block;
	}
}

void Game::run () {
	running = true;

	setup_spheres ();

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
	}

	engine->simulation->task_scheduler.stop ();
}

void Game::write_game_state (RenderState* render_state) {
	static std::shared_ptr<Mesh> sphere_mesh = std::make_shared<Mesh> (
		Sphere::generate (15.0f, 20, 20)
	);
	sphere_mesh->to_gpu ();

	static InstanceBatch sphere_instances;
	static Material material = {
		.name = "test_material",
		.pipeline_name = "lit_opaque_backcull",
		.shader_name = "anomaly"
	};

	Drawable sphere{};
	sphere.mesh = sphere_mesh.get ();
	sphere.material = &material;
	sphere.instance_batch = &sphere_instances;

	sphere.instance_batch->blocks.resize (NUM_INSTANCES);

	std::memcpy (
		sphere.instance_batch->blocks.data (), instances.data (),
		sphere.instance_batch->blocks.size () * sizeof (Block)
	);

	render_state->drawables.push_back (sphere);
}
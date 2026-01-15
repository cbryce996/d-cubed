#include "engine.h"

#include "object/demos/instancing.h"
#include "object/demos/wave.h"
#include "runtime/clock.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <chrono>
#include <iostream>

Engine::Engine () {
	if (!SDL_Init (SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError () << "\n";
		return;
	}

	SDL_SetLogPriority (SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_TRACE);

	window = SDL_CreateWindow ("Game", 1280, 720, SDL_WINDOW_RESIZABLE);

	// SDL_SetWindowFullscreen (window, true);

	gpu_device = SDL_CreateGPUDevice (
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL
			| SDL_GPU_SHADERFORMAT_MSL,
		true, "metal"
	);

	if (!gpu_device) {
		std::cerr << "SDL_CreateGPUDevice failed: " << SDL_GetError () << "\n";
		return;
	}

	if (!SDL_ClaimWindowForGPUDevice (gpu_device, window)) {
		std::cerr << "SDL_ClaimWindowForGPUDevice failed: " << SDL_GetError ()
				  << "\n";
		return;
	}

	std::shared_ptr<ShaderManager> shader_manager
		= std::make_shared<ShaderManager> (gpu_device);
	std::shared_ptr<PipelineManager> pipeline_manager
		= std::make_shared<PipelineManager> (
			gpu_device, window, shader_manager
		);
	std::shared_ptr<BufferManager> buffer_manager
		= std::make_shared<BufferManager> (gpu_device);

	Camera camera{};
	camera.name = "main";
	camera.transform.position = glm::vec3 (0.0f, 0.0f, 3.0f);
	camera.transform.rotation = glm::quat (
		glm::vec3 (0.0f, glm::radians (180.0f), 0.0f)
	);
	camera.transform.scale = glm::vec3 (1.0f);
	camera.lens.fov = 100.0f;
	camera.lens.aspect = 16.0f / 9.0f;
	camera.lens.near_clip = 0.1f;
	camera.lens.far_clip = 300.0f;
	camera.move_speed = 0.1f;
	camera.look_sensitivity = 0.1f;

	std::shared_ptr<CameraManager> camera_manager
		= std::make_shared<CameraManager> (camera);
	std::shared_ptr<AssetManager> asset_manager
		= std::make_shared<AssetManager> ();

	render = std::make_unique<RenderManager> (
		gpu_device, window, shader_manager, pipeline_manager, buffer_manager,
		camera_manager, asset_manager
	);

	runtime = std::make_unique<Runtime> ();

	SDL_SetWindowRelativeMouseMode (window, true);
}

Engine::~Engine () {
	SDL_DestroyGPUDevice (gpu_device);
	SDL_DestroyWindow (window);
	SDL_Quit ();
}

void Engine::run () {
	running = true;

	runtime->task_scheduler.start ();
	Clock clock (60);

	while (running) {
		commit_scene_change ();
		float dt = clock.begin_frame ();

		SDL_Event e;
		while (SDL_PollEvent (&e)) {
			if (e.type == SDL_EVENT_QUIT)
				running = false;
			if (e.type == SDL_EVENT_WINDOW_RESIZED)
				render->resize (e.window.data1, e.window.data2);
		}

		input.poll ();

		const KeyboardInput& keyboard = input.get_keyboard_input ();

		if (keyboard.keys[SDL_SCANCODE_ESCAPE])
			running = false;
		if (keyboard.keys[SDL_SCANCODE_1]) {
			std::unique_ptr<Scene> scene = std::make_unique<Scene> ();
			scene->add_object (std::make_unique<InstancingDemo> ());
			request_scene (std::move (scene));
		}
		if (keyboard.keys[SDL_SCANCODE_2]) {
			std::unique_ptr<Scene> scene = std::make_unique<Scene> ();

			float spacing = 0.25f;

			scene->add_object (
				std::make_unique<Wave> (
					glm::vec3 (0, 0, 0), 0.0f, 0.0f, "wave1"
				)
			);

			scene->add_object (
				std::make_unique<Wave> (
					glm::vec3 (spacing, -20, spacing), glm::radians (45.0f),
					0.0f, "wave2"
				)
			);

			request_scene (std::move (scene));
		}

		const MouseInput& mouse = input.get_mouse_input ();

		render->camera_manager->update_camera_position (
			clock.fixed_dt_ms, keyboard.keys
		);
		render->camera_manager->update_camera_look (&mouse);

		while (clock.should_step_simulation ()) {
			runtime->update (clock.fixed_dt_ms);

			if (active_scene)
				active_scene->update (
					clock.fixed_dt_ms, runtime->simulation_time_ms
				);

			clock.consume_simulation_step ();
		}

		RenderState state{};
		if (active_scene)
			active_scene->collect_drawables (state);

		render->render (&state, runtime->simulation_time_ms);

		clock.end_frame ();
	}

	runtime->task_scheduler.stop ();
}

void Engine::request_scene (std::unique_ptr<Scene> in_scene) {
	pending_scene = std::move (in_scene);
}

void Engine::commit_scene_change () {
	if (!pending_scene)
		return;

	if (active_scene)
		active_scene->on_unload ();

	active_scene = std::move (pending_scene);

	if (active_scene)
		active_scene->on_load ();
}

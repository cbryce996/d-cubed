#include "engine.h"

#include <SDL3/SDL.h>
#include <inputs/input.h>

#include <chrono>
#include <glm/glm.hpp>
#include <iostream>

#include "../game/game.h"

Engine::Engine() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError() << "\n";
		return;
	}

	window = SDL_CreateWindow("Game", 1280, 720, SDL_WINDOW_RESIZABLE);

	gpu_device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL |
			SDL_GPU_SHADERFORMAT_MSL,
		true,
		"metal"
	);

	if (!gpu_device) {
		std::cerr << "SDL_CreateGPUDevice failed: " << SDL_GetError() << "\n";
		return;
	}

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		std::cerr << "SDL_ClaimWindowForGPUDevice failed: " << SDL_GetError()
				  << "\n";
		return;
	}

	std::shared_ptr<ShaderManager> shader_manager =
		std::make_shared<ShaderManager>(gpu_device);
	std::shared_ptr<PipelineManager> pipeline_manager =
		std::make_shared<PipelineManager>(gpu_device, window, shader_manager);
	std::shared_ptr<BufferManager> buffer_manager =
		std::make_shared<BufferManager>(gpu_device);
	std::shared_ptr<CameraManager> camera_manager =
		std::make_shared<CameraManager>();
	std::shared_ptr<AssetManager> asset_manager =
		std::make_shared<AssetManager>();

	render = std::make_unique<RenderManager>(
		gpu_device,
		window,
		shader_manager,
		pipeline_manager,
		buffer_manager,
		camera_manager,
		asset_manager
	);

	game = std::make_unique<GameManager>(asset_manager, shader_manager);

	SDL_SetWindowRelativeMouseMode(window, true);
}

Engine::~Engine() {
	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Engine::run() {
	using clock = std::chrono::high_resolution_clock;

	static_assert(sizeof(glm::vec3) == 12, "vec3 isn't 12 bytes?");
	static_assert(sizeof(Vertex) == 36, "Vertex struct is misaligned!");

	running = true;
	task_scheduler.start();

	constexpr int TARGET_FPS = 60;
	constexpr int FRAME_DELAY = 1000 / TARGET_FPS;

	int frame_counter = 0;
	float accumulated_frame_time_ms = 0.0f;
	auto last_stat_time = clock::now();
	auto last_sim_time = clock::now();

	Camera camera{};
	camera.name = "main";

	camera.transform.position = glm::vec3(0.0f, 0.0f, 3.0f);
	camera.transform.rotation =
		glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
	camera.transform.scale = glm::vec3(1.0f);

	camera.lens.fov = 90.0f;
	camera.lens.aspect = 16.0f / 9.0f;
	camera.lens.near_clip = 0.1f;
	camera.lens.far_clip = 100.0f;

	camera.move_speed = 0.2f;
	camera.look_sensitivity = 0.1f;

	render->camera_manager->add_camera(camera);
	render->camera_manager->set_active_camera(camera);

	while (running) {
		auto frame_start = clock::now();

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT)
				running = false;
		}

		auto current_time = clock::now();
		const float delta_time_ms = std::chrono::duration<float, std::milli>(
										current_time - last_sim_time
		)
										.count();
		last_sim_time = current_time;
		accumulated_frame_time_ms += delta_time_ms;

		input.poll();
		GameManager::handle_input(input);
		const KeyboardInput* keyboard_input = &input.get_keyboard_input();
		const MouseInput* mouse_input = &input.get_mouse_input();

		if (keyboard_input->keys[SDL_SCANCODE_ESCAPE])
			running = false;

		game->update(delta_time_ms, task_scheduler, input);
		game->write_render_state(accumulated_frame_time_ms);

		render->camera_manager->update_camera_position(
			delta_time_ms,
			keyboard_input->keys
		);
		render->camera_manager->update_camera_look(mouse_input);
		render->render(game->render_state);

		auto frame_end = clock::now();
		float frame_time_ms =
			std::chrono::duration<float, std::milli>(frame_end - frame_start)
				.count();

		if (frame_time_ms < FRAME_DELAY) {
			SDL_Delay(static_cast<Uint32>(FRAME_DELAY - frame_time_ms));

			auto delayed_end = clock::now();
			frame_time_ms = std::chrono::duration<float, std::milli>(
								delayed_end - frame_start
			)
								.count();
		}

		accumulated_frame_time_ms += frame_time_ms;
		frame_counter++;

		if (std::chrono::duration_cast<std::chrono::seconds>(
				frame_start - last_stat_time
			)
				.count() >= 1) {
			float avg_frame_time = accumulated_frame_time_ms / frame_counter;
			float avg_fps = 1000.0f / avg_frame_time;

			std::cout << "[Perf] Avg Frame Time: " << avg_frame_time
					  << " ms | Avg FPS: " << avg_fps << "\n";

			accumulated_frame_time_ms = 0.0f;
			frame_counter = 0;
			last_stat_time = frame_start;
		}
	}

	task_scheduler.stop();
}
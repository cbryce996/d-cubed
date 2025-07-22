#include "engine.h"

#include <chrono>
#include <iostream>

#include "../game/game.h"

Engine::Engine()
	: window(SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0)),
	  render(*window)
{
	SDL_Init(SDL_INIT_VIDEO);
}

Engine::~Engine() = default;


void Engine::run() {
	using clock = std::chrono::high_resolution_clock;

	GameManager game;
	running = true;
	task_scheduler.start();

	constexpr int TARGET_FPS = 60;
	constexpr int FRAME_DELAY = 1000 / TARGET_FPS;

	int frame_counter = 0;
	float accumulated_frame_time_ms = 0.0f;
	auto last_stat_time = clock::now();

	auto last_sim_time = clock::now();

	while (running) {
		auto frame_start = clock::now();

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				running = false;
		}

		auto current_time = clock::now();
		const float delta_time_ms = std::chrono::duration<float, std::milli>(current_time - last_sim_time).count();
		last_sim_time = current_time;

		input.poll();
		game.handle_input(input);
		game.update(delta_time_ms, task_scheduler, input);

		RenderState render_state;
		game.write_render_state(render_state);
		render.render_state(render_state);

		auto frame_end = clock::now();
		float frame_time_ms = std::chrono::duration<float, std::milli>(frame_end - frame_start).count();

		if (frame_time_ms < FRAME_DELAY) {
			SDL_Delay(static_cast<Uint32>(FRAME_DELAY - frame_time_ms));

			auto delayed_end = clock::now();
			frame_time_ms = std::chrono::duration<float, std::milli>(delayed_end - frame_start).count();
		}

		accumulated_frame_time_ms += frame_time_ms;
		frame_counter++;

		if (std::chrono::duration_cast<std::chrono::seconds>(frame_start - last_stat_time).count() >= 1) {
			float avg_frame_time = accumulated_frame_time_ms / frame_counter;
			float avg_fps = 1000.0f / avg_frame_time;

			std::cout << "[Perf] Avg Frame Time: " << avg_frame_time << " ms | Avg FPS: " << avg_fps << "\n";

			accumulated_frame_time_ms = 0.0f;
			frame_counter = 0;
			last_stat_time = frame_start;
		}
	}

	task_scheduler.stop();
}

void Engine::shutdown() const {
	SDL_DestroyWindow(window);
	SDL_Quit();
}

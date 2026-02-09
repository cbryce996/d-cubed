#include "engine.h"

#include "assets/asset.h"
#include "cameras/camera.h"
#include "editor/editor.h"
#include "entity/prefabs/spheres.h"
#include "render/buffers/buffer.h"
#include "render/frame/frame.h"
#include "render/pipelines/sdl/factory.h"
#include "render/render.h"
#include "render/shaders/shader.h"
#include "runtime/clock.h"
#include "runtime/runtime.h"
#include "scene.h"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

#include <iostream>

Engine::Engine () {
	if (!SDL_Init (SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError () << "\n";
		return;
	}

	SDL_SetLogPriority (SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_TRACE);

	window = SDL_CreateWindow (
		"Game", 2560, 1600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

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
			std::make_shared<SDLPipelineFactory> (
				gpu_device, window, shader_manager
			)
		);
	std::shared_ptr<BufferManager> buffer_manager
		= std::make_shared<BufferManager> (gpu_device);

	std::shared_ptr<AssetManager> asset_manager
		= std::make_shared<AssetManager> ();
	std::shared_ptr<EditorManager> editor_manager
		= std::make_shared<EditorManager> ();
	std::shared_ptr<FrameManager> frame_manager
		= std::make_shared<FrameManager> ();

	std::shared_ptr<ResourceManager> resource_manager
		= std::make_shared<ResourceManager> (Target{});

	render = std::make_unique<RenderManager> (
		gpu_device, window, shader_manager, pipeline_manager, buffer_manager,
		asset_manager, editor_manager, frame_manager, resource_manager
	);

	runtime = std::make_unique<Runtime> ();
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

	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	ImGuiIO& io = ImGui::GetIO ();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	int ww, wh;
	SDL_GetWindowSize (window, &ww, &wh);
	int dw, dh;
	SDL_GetWindowSizeInPixels (window, &dw, &dh);

	io.DisplaySize = ImVec2 (static_cast<float> (ww), static_cast<float> (wh));
	io.DisplayFramebufferScale = ImVec2 (
		static_cast<float> (dw) / static_cast<float> (ww),
		static_cast<float> (dh) / static_cast<float> (wh)
	);

	float dpi = io.DisplayFramebufferScale.x;

	ImFontConfig cfg{};
	cfg.SizePixels = 14.0f * dpi;
	// optional knobs:
	cfg.OversampleH = 2;
	cfg.OversampleV = 2;
	cfg.PixelSnapH = true;

	io.Fonts->AddFontDefault (&cfg);

	// Keep “visual size” the same in UI points, but crisp
	io.FontGlobalScale = 0.9f / dpi;

	ImGui::StyleColorsDark ();

	ImGui_ImplSDL3_InitForSDLGPU (window);

	ImGui_ImplSDLGPU3_InitInfo init_info = {
		gpu_device,
		SDL_GetGPUSwapchainTextureFormat (gpu_device, window),
		SDL_GPU_SAMPLECOUNT_1,
	};

	ImGui_ImplSDLGPU3_Init (&init_info);

	while (running) {
		commit_scene_change ();
		float dt = clock.begin_frame ();

		if (render->editor_manager->editor_mode == Editing) {
			SDL_SetWindowRelativeMouseMode (window, false);
		} else {
			SDL_SetWindowRelativeMouseMode (window, true);
		}

		SDL_Event e;
		while (SDL_PollEvent (&e)) {
			ImGui_ImplSDL3_ProcessEvent (&e);
			if (e.type == SDL_EVENT_QUIT)
				running = false;
			if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
				if (e.key.key == SDL_GetKeyFromName ("I")) {
					auto& editor = *render->editor_manager;

					editor.editor_mode = (editor.editor_mode == Editing)
											 ? Running
											 : Editing;
				}
			}
		}

		input.poll ();

		const KeyboardInput& keyboard_input = input.get_keyboard_input ();
		MouseInput mouse_input = input.get_mouse_input ();

		if (keyboard_input.keys[SDL_SCANCODE_ESCAPE])
			running = false;
		if (keyboard_input.keys[SDL_SCANCODE_1]) {
			std::unique_ptr<Scene> scene = std::make_unique<Scene> ();

			scene->add_entity (
				std::make_unique<Spheres> (
					glm::vec3 (0, 0, 0), 0.0f, 0.0f, "wave1"
				)
			);

			request_scene (std::move (scene));
		}

		while (clock.should_step_simulation ()) {
			runtime->update (clock.fixed_dt_ms);

			if (active_scene)
				active_scene->update (
					clock.fixed_dt_ms, runtime->simulation_time_ms
				);

			clock.consume_simulation_step ();
		}

		RenderState state{};
		state.scene = active_scene.get ();
		if (active_scene)
			active_scene->collect_drawables (state);

		render->render (
			state, keyboard_input, mouse_input, runtime->simulation_time_ms
		);

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

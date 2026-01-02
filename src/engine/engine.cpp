#include "engine.h"

#include <SDL3/SDL.h>

#include <chrono>
#include <iostream>

Engine::Engine () {
	if (!SDL_Init (SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError () << "\n";
		return;
	}

	SDL_SetLogPriority (SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_TRACE);

	window = SDL_CreateWindow ("Game", 1280, 720, SDL_WINDOW_RESIZABLE);

	SDL_SetWindowFullscreen (window, true);

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

	simulation = std::make_unique<Simulation> ();

	SDL_SetWindowRelativeMouseMode (window, true);
}

Engine::~Engine () {
	SDL_DestroyGPUDevice (gpu_device);
	SDL_DestroyWindow (window);
	SDL_Quit ();
}
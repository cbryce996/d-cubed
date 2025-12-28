#include "engine.h"

#include <SDL3/SDL.h>
#include <inputs/input.h>

#include <chrono>
#include <glm/glm.hpp>
#include <iostream>

#include "../game/game.h"

Engine::Engine () {
	if (!SDL_Init (SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError () << "\n";
		return;
	}

	window = SDL_CreateWindow ("Game", 1280, 720, SDL_WINDOW_RESIZABLE);

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
	std::shared_ptr<CameraManager> camera_manager
		= std::make_shared<CameraManager> ();
	std::shared_ptr<AssetManager> asset_manager
		= std::make_shared<AssetManager> ();

	render = std::make_unique<RenderManager> (
		gpu_device, window, shader_manager, pipeline_manager, buffer_manager,
		camera_manager, asset_manager
	);

	SDL_SetWindowRelativeMouseMode (window, true);
}

Engine::~Engine () {
	SDL_DestroyGPUDevice (gpu_device);
	SDL_DestroyWindow (window);
	SDL_Quit ();
}
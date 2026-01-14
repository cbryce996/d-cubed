#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>

#include "assets/asset.h"
#include "cameras/camera.h"
#include "inputs/input.h"
#include "render/render.h"
#include "runtime/runtime.h"

class Engine {
  public:
	Engine ();
	~Engine ();

	InputManager input;

	std::unique_ptr<Runtime> runtime;
	std::unique_ptr<CameraManager> camera;
	std::unique_ptr<RenderManager> render;
	std::unique_ptr<AssetManager> asset;

  private:
	SDL_GPUDevice* gpu_device = nullptr;
	SDL_Window* window = nullptr;
};

#endif // ENGINE_H

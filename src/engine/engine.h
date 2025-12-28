#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>

#include "assets/asset.h"
#include "cameras/camera.h"
#include "inputs/input.h"
#include "render/render.h"
#include "tasks/tasks.h"

struct ItemInstance {
	SDL_Rect rect;
};

struct CraftingInstance {
	SDL_Rect target_rect;
	float progress_ms = 0.0f;
	float duration_ms = 2000.0f;
};

class Engine {
  public:
	Engine ();
	~Engine ();

	void run ();

	TaskScheduler task_scheduler;
	InputManager input;

	std::unique_ptr<CameraManager> camera;
	std::unique_ptr<RenderManager> render;
	std::unique_ptr<AssetManager> asset;

  private:
	SDL_GPUDevice* gpu_device = nullptr;
	SDL_Window* window = nullptr;
};

#endif // ENGINE_H

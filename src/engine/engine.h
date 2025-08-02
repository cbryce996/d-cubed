#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>

#include "camera.h"
#include "game/game.h"
#include "inputs.h"
#include "render/render.h"
#include "tasks.h"

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
	Engine();
	~Engine();

	void run();

   private:
	bool running = false;

	SDL_GPUDevice* gpu_device = nullptr;
	SDL_Window* window = nullptr;

	TaskScheduler task_scheduler;
	InputManager input;
	GameManager game;

	std::unique_ptr<CameraManager> camera;
	std::unique_ptr<RenderManager> render;
};

#endif	// ENGINE_H
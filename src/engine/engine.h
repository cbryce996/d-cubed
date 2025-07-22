#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>

#include "inputs.h"
#include "tasks.h"
#include "game/game.h"
#include "render/render.h"

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
	void shutdown() const;

	TaskScheduler task_scheduler;

   private:
	bool running = false;

	SDL_Window* window = nullptr;

	GameManager game;
	RenderManager render;
	InputManager input;
};

#endif	// ENGINE_H
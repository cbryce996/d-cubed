#ifndef ENGINE_H
#define ENGINE_H

#include "inputs/input.h"

#include <SDL3/SDL.h>
#include <memory>

class AssetManager;
class RenderManager;
class CameraManager;
class Runtime;
class Scene;

class Engine {
  public:
	Engine ();
	~Engine ();

	void run ();
	void request_scene (std::unique_ptr<Scene> in_scene);
	void commit_scene_change ();

	InputManager input;

	std::unique_ptr<Scene> active_scene = nullptr;
	std::unique_ptr<Scene> pending_scene = nullptr;

	std::unique_ptr<Runtime> runtime;
	std::unique_ptr<RenderManager> render;
	std::unique_ptr<AssetManager> asset;

  private:
	SDL_GPUDevice* gpu_device = nullptr;
	SDL_Window* window = nullptr;

	bool running = false;
};

#endif // ENGINE_H

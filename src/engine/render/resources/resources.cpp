#include "resources.h"

#include <SDL3/SDL.h>

ResourceManager::ResourceManager (const Target& viewport_target)
	: viewport_target (viewport_target) {}

void ResourceManager::resize_viewport (
	SDL_GPUDevice* device, const int in_width, const int in_height
) {
	if (in_width <= 0 || in_height <= 0)
		return;

	if (viewport_target.valid () && viewport_target.width == in_width
		&& viewport_target.height == in_height)
		return;

	SDL_WaitForGPUIdle (device);

	viewport_target.destroy (device);
	viewport_target.create (
		device, in_width, in_height, SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,
		SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER
	);
}

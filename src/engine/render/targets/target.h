#ifndef TARGETS_H
#define TARGETS_H

#include <SDL3/SDL.h>

class Target {
  public:
	static constexpr int BufferCount = 2;

	SDL_GPUTexture* textures[BufferCount] = {nullptr, nullptr};

	int width = 0;
	int height = 0;

	int write_index = 0;
	int read_index = 1;

	[[nodiscard]] bool valid () const;

	[[nodiscard]] SDL_GPUTexture* write () const;
	[[nodiscard]] SDL_GPUTexture* read () const;

	void swap ();
	void reset ();

	void destroy (SDL_GPUDevice* device);
	void create (
		SDL_GPUDevice* device, int width, int height,
		SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage
	);
};

#endif // TARGETS_H

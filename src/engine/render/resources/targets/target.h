#ifndef TARGETS_H
#define TARGETS_H

#include <SDL3/SDL.h>
#include <algorithm>

class Target {
  public:
	Target (int height, int width);

	static constexpr int BufferCount = 2;

	SDL_GPUTexture* textures[BufferCount] = {nullptr, nullptr};

	float aspect_ratio
		= std::max (static_cast<float> (width), static_cast<float> (height))
		  / std::min (static_cast<float> (width), static_cast<float> (height));

	int width;
	int height;

	int write_index = 0;
	int read_index = 1;

	[[nodiscard]] bool valid () const;

	[[nodiscard]] SDL_GPUTexture* write () const;
	[[nodiscard]] SDL_GPUTexture* read () const;

	void swap ();
	void reset ();

	void destroy (SDL_GPUDevice* device);
	void create (
		SDL_GPUDevice* device, int in_width, int in_height,
		SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage
	);
};

#endif // TARGETS_H

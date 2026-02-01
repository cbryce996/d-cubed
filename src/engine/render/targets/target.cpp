#include "target.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <future>

bool Target::valid () const {
	return textures[0] && textures[1] && width > 0 && height > 0;
}

SDL_GPUTexture* Target::write () const { return textures[write_index]; }

SDL_GPUTexture* Target::read () const { return textures[read_index]; }

void Target::swap () { std::swap (write_index, read_index); }

void Target::reset () {
	textures[0] = nullptr;
	textures[1] = nullptr;
	width = height = 0;
	write_index = 0;
	read_index = 1;
}

void Target::destroy (SDL_GPUDevice* device) {
	for (int i = 0; i < BufferCount; ++i) {
		if (textures[i]) {
			SDL_ReleaseGPUTexture (device, textures[i]);
			textures[i] = nullptr;
		}
	}
	reset ();
}

void Target::create (
	SDL_GPUDevice* device, int in_width, int in_height,
	SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage
) {
	destroy (device);

	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.width = in_width;
	info.height = in_height;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.format = format;
	info.usage = usage;

	for (int i = 0; i < BufferCount; ++i) {
		textures[i] = SDL_CreateGPUTexture (device, &info);
		assert (textures[i]);
	}

	width = in_width;
	height = in_height;
	write_index = 0;
	read_index = 1;
}
#include "factory.h"

void SDLTextureFactory::destroy (SDL_GPUDevice* device) {
	// TODO
}

void SDLTextureFactory::create (
	SDL_GPUDevice* device, int in_width, int in_height,
	const SDL_GPUTextureFormat format, const SDL_GPUTextureUsageFlags usage
) {
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
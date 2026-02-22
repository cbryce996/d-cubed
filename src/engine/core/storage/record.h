#ifndef RECORD_H
#define RECORD_H

#include <SDL3/SDL_gpu.h>

#include "render/textures/texture.h"

struct TextureRecord {
	SDL_GPUTexture* tex = nullptr;

	TextureState state{};
	uint64_t approx_bytes = 0;
	bool is_target = false;
};

#endif // RECORD_H

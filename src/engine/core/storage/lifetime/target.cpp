#include "target.h"

#include <SDL3/SDL.h>
#include <algorithm>

TextureTarget::TextureTarget () {}

bool TextureTarget::valid () const {
	return textures[0] && textures[1] && width > 0 && height > 0;
}

SDL_GPUTexture* TextureTarget::write () const { return textures[write_index]; }
SDL_GPUTexture* TextureTarget::read () const { return textures[read_index]; }

float TextureTarget::aspect_ratio () const {
	return (height > 0)
			   ? (static_cast<float> (width) / static_cast<float> (height))
			   : 1.0f;
}

void TextureTarget::swap () { std::swap (write_index, read_index); }

void TextureTarget::reset () {
	textures[0] = nullptr;
	textures[1] = nullptr;
	width = height = 0;
	write_index = 0;
	read_index = 1;
}
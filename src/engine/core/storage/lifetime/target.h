#ifndef TARGET_H
#define TARGET_H

#include <SDL3/SDL_gpu.h>

class TextureTarget {
  public:
	TextureTarget ();

	static constexpr int BufferCount = 2;

	SDL_GPUTexture* textures[BufferCount] = {nullptr, nullptr};

	int width = 0;
	int height = 0;

	int write_index = 0;
	int read_index = 1;

	[[nodiscard]] bool valid () const;

	[[nodiscard]] SDL_GPUTexture* write () const;
	[[nodiscard]] SDL_GPUTexture* read () const;

	[[nodiscard]] float aspect_ratio () const;

	void swap ();
	void reset ();
};

#endif // TARGET_H

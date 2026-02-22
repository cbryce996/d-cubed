#ifndef SDL_TEXTURE_FACTORY_H
#define SDL_TEXTURE_FACTORY_H

#include <SDL3/SDL_gpu.h>

#include "core/factory.h"
#include "core/storage/maps/slot.h"

struct TextureState;
struct TextureRegistryStats;

class SDLTextureFactory final : public IFactory<TextureState> {
  public:
	SDLTextureFactory (
		SDL_GPUDevice* device, DenseSlotMapStorage& storage,
		TextureRegistryStats* stats
	)
		: IFactory (storage), device (device), stats (stats) {}

	Handle create (const TextureState& state) override;
	void destroy (Handle handle) override;

  private:
	SDL_GPUDevice* device = nullptr;
	TextureRegistryStats* stats = nullptr;

	static uint32_t bytes_per_pixel (TextureFormat format);
	static uint64_t estimate_bytes (const TextureState& state);
};

#endif // SDL_TEXTURE_FACTORY_H

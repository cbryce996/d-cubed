#ifndef SDL_TEXTURE_FACTORY_H
#define SDL_TEXTURE_FACTORY_H

#include <SDL3/SDL.h>
#include <memory>

#include "resources/factory.h"
#include "resources/state.h"

class DenseSlotMapStorage;
struct TextureState;
class IStorage;
struct Handle;
class ShaderManager;

class SDLTextureFactory final
	: public IFactory<TextureState, DenseSlotMapStorage> {
  public:
	SDLTextureFactory (SDL_GPUDevice* device, SDL_Window* window);
	~SDLTextureFactory () override;

	Handle create (
		const IState<TextureState>& state, DenseSlotMapStorage& storage
	) override;
	void destroy (Handle handle, DenseSlotMapStorage& storage) override;

  private:
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	std::shared_ptr<ShaderManager> shader_manager;
};

#endif // SDL_TEXTURE_FACTORY_H

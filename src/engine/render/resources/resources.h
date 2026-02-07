#ifndef RESOURCES_H
#define RESOURCES_H
#include "targets/target.h"

class ResourceManager {
  public:
	explicit ResourceManager (const Target& viewport_target);

	Target viewport_target;

	void resize_viewport (SDL_GPUDevice* device, int in_width, int in_height);
};

#endif // RESOURCES_H

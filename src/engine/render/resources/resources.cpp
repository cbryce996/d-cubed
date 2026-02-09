#include "resources.h"

#include <SDL3/SDL.h>
#include <algorithm>

ResourceManager::ResourceManager (const Target& viewport_target)
	: viewport_target (viewport_target) {}

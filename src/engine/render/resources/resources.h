#ifndef RESOURCES_H
#define RESOURCES_H
#include "targets/target.h"

class ResourceManager {
  public:
	explicit ResourceManager (const Target& viewport_target);

	Target viewport_target;
};

#endif // RESOURCES_H

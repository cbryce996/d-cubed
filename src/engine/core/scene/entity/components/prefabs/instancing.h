#ifndef INSTANCING_H
#define INSTANCING_H

#include "entity/components/component.h"
#include "entity/entity.h"
#include "render/memory.h"

#include <vector>

class InstancingComponent final : public IEntityComponent {
  public:
	std::vector<Transform> instances;
	void pack (const glm::mat4& base_world, std::vector<Block>& out) const;
};

#endif // INSTANCING_H

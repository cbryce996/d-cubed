#ifndef FLOOR_H
#define FLOOR_H

#include "entity/entity.h"

#include <string>

struct MaterialInstance;
struct MeshInstance;
struct Block;

class StaticEntity final : public IEntity {
  public:
	explicit StaticEntity (
		std::string name = "Static", MeshInstance* mesh = nullptr,
		MaterialInstance* material = nullptr, const Transform& transform = {},
		const Transform& world_transform = {}
	);

	void on_load () override;
	void on_unload () override;

	void update (float dt_ms, float sim_time_ms) override;
};

#endif //  FLOOR_H

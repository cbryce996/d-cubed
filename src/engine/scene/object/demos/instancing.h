#ifndef INSTANCING_H
#define INSTANCING_H

#include "object/object.h"
#include "render/block.h"

#include <string>
#include <vector>

struct Material;

class InstancingDemo final : public ISceneObject {
  public:
	InstancingDemo ();

	std::string name;

	void on_load () override;
	void on_unload () override;
	void update (float dt_ms, float sim_time_ms) override;

	void collect_drawables (RenderState& out_render_state) override;

  private:
	void setup_instances (
		std::vector<Block>& out_instances, int count, uint32_t seed
	);

	static constexpr int NUM_SPHERES = 1000;
	static constexpr int NUM_CUBES = 1000;

	std::vector<Block> sphere_instances;
	std::vector<Block> cube_instances;
};

#endif // INSTANCING_H

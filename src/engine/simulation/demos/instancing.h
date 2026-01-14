#ifndef INSTANCING_H
#define INSTANCING_H

#include "simulation/simulation.h"

class InstancingDemo final : public ISimulation {
  public:
	InstancingDemo () = default;

	void on_load () override;
	void on_unload () override;

	void fixed_update (float dt_ms, float sim_time_ms) override;
	void build_render_state (RenderState& out) override;

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

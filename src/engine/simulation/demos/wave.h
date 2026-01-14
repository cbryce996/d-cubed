#ifndef WAVE_H
#define WAVE_H
#include "simulation/simulation.h"

class WaveDemo final : public ISimulation {
  public:
	void on_load () override;
	void on_unload () override;
	void fixed_update (float dt_ms, float sim_time_ms) override;
	void build_render_state (RenderState& out) override;

  private:
	void setup_grid ();

	std::vector<Block> instances;

	static constexpr int GRID_SIZE = 128;
	static constexpr float SPACING = 2.0f;
};

#endif // WAVE_H

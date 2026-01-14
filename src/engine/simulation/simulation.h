#ifndef GAME_H
#define GAME_H

#include "render/render.h"

class ISimulation {
  public:
	virtual ~ISimulation () = default;

	virtual void on_load () {}
	virtual void on_unload () {}

	virtual void fixed_update (float dt_ms, float sim_time_ms) = 0;
	virtual void build_render_state (RenderState& out) = 0;
};

#endif // GAME_H

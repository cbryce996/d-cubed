#ifndef OBJECT_H
#define OBJECT_H

#include <string>

struct RenderState;

class ISceneObject {
  public:
	explicit ISceneObject (std::string name);
	virtual ~ISceneObject ();

	std::string name;

	virtual void on_load () {}
	virtual void on_unload () {}

	virtual void update (float dt_ms, float sim_time_ms) = 0;
	virtual void collect_drawables (RenderState& out_render_state);
};

#endif // OBJECT_H

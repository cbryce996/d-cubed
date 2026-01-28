#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <string>

class IEntity;
struct RenderState;

struct SceneLighting {
	glm::vec3 main_light_position;
	glm::vec3 main_light_color;
	float ambient_intensity = 0.3f;
};

class Scene {
  public:
	void on_load ();
	void on_unload ();

	void update (float dt_ms, float sim_time_ms);
	void collect_drawables (RenderState& out_render_state);

	void add_entity (std::unique_ptr<IEntity> object);

	std::unordered_map<std::string, std::unique_ptr<IEntity>> scene_entities;

  private:
	bool loaded = false;
};

#endif // SCENE_H

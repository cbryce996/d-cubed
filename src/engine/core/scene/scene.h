#ifndef SCENE_H
#define SCENE_H

#include "entity/prefabs/static.h"

#include <glm/glm.hpp>
#include <string>

#include "core/camera/camera.h"

class IEntity;
struct RenderState;

struct SceneLighting {
	glm::vec3 main_light_position;
	glm::vec3 main_light_color;
	float ambient_intensity = 0.3f;
};

class Scene {
  public:
	Scene ();
	void on_load ();
	void on_unload ();

	void update (float dt_ms, float sim_time_ms);
	void collect_drawables (RenderState& out_render_state);

	void add_entity (std::unique_ptr<IEntity> entity);

	std::unordered_map<std::string, std::unique_ptr<IEntity>> scene_entities;
	std::unique_ptr<CameraManager> camera_manager;

  private:
	bool loaded = false;
};

#endif // SCENE_H

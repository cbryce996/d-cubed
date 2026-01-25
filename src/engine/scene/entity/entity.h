#ifndef OBJECT_H
#define OBJECT_H

#include "mesh/mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct MaterialInstance;
struct MeshInstance;
struct RenderState;

struct Transform {
	glm::vec3 position = {0.0f, 0.0f, 5.0f};
	glm::quat rotation = glm::quat (0.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

struct InstancingComponent {
	std::vector<Transform> instances;
};

class IEntity {
  public:
	explicit IEntity (std::string name);
	virtual ~IEntity ();
	virtual void pack_instances (std::vector<Block>& out_blocks) const = 0;

	std::string name;
	Transform transform;
	MeshInstance* mesh = nullptr;
	MaterialInstance* material = nullptr;

	std::unique_ptr<InstancingComponent> instancing_component;

	virtual void on_load () {}
	virtual void on_unload () {}

	virtual void update (float dt_ms, float sim_time_ms) = 0;
};

#endif // OBJECT_H

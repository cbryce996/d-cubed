#ifndef OBJECT_H
#define OBJECT_H

#include "components/component.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <set>
#include <string>

struct MaterialInstance;
struct MeshInstance;
struct RenderState;

struct Transform {
	glm::vec3 position = {0.0f, 0.0f, 5.0f};
	glm::quat rotation = glm::quat (0.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

#include <memory>
#include <typeindex>
#include <unordered_map>

class IEntity {
  public:
	explicit IEntity (std::string name);
	virtual ~IEntity ();

	std::string name;
	Transform transform;
	MeshInstance* mesh = nullptr;
	MaterialInstance* material = nullptr;

	virtual void on_load () {}
	virtual void on_unload () {}
	virtual void update (float dt_ms, float sim_time_ms) = 0;

	template <typename T, typename... Args> T& add_component (Args&&... args) {
		static_assert (std::is_base_of_v<IEntityComponent, T>);
		auto component = std::make_unique<T> (std::forward<Args> (args)...);
		T& reference = *component;
		components[typeid (T)] = std::move (component);
		reference.on_attach ();
		return reference;
	}

	template <typename T> T* get_component () {
		const auto iterator = components.find (typeid (T));
		if (iterator == components.end ())
			return nullptr;
		return static_cast<T*> (iterator->second.get ());
	}

	template <typename T> [[nodiscard]] bool has_component () const {
		return components.contains (typeid (T));
	}

  private:
	std::unordered_map<std::type_index, std::unique_ptr<IEntityComponent>>
		components;
};

#endif // OBJECT_H

#include "assets/asset.h"
#include "engine.h"
#include "entity/prefabs/spheres.h"
#include "entity/prefabs/static.h"
#include "maths/geometry/cube.h"
#include "maths/geometry/plane.h"
#include "maths/geometry/sphere.h"
#include "mesh/mesh.h"
#include "render/material.h"
#include "scene.h"

#include <memory>

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();
	std::unique_ptr<Scene> scene = std::make_unique<Scene> ();

	static std::shared_ptr<MeshInstance> cube = std::make_shared<MeshInstance> (
		Cube::generate (5.0f)
	);
	static std::shared_ptr<MeshInstance> plane
		= std::make_shared<MeshInstance> (Plane::generate (50.0f));
	static std::shared_ptr<MeshInstance> sphere
		= std::make_shared<MeshInstance> (Sphere::generate (2.5f, 20, 20));

	std::unique_ptr<StaticEntity> static_plane
		= std::make_unique<StaticEntity> (
			"static_plane", plane.get (), &Materials::Geometry
		);
	std::unique_ptr<StaticEntity> static_cube = std::make_unique<StaticEntity> (
		"static_cube", cube.get (), &Materials::Geometry,
		Transform{glm::vec3 (-5.0f, -2.5f, -10.0f)}
	);
	static_cube->set_parent (std::move (static_plane).get ());
	std::unique_ptr<StaticEntity> static_sphere
		= std::make_unique<StaticEntity> (
			"static_sphere", sphere.get (), &Materials::Geometry,
			Transform{glm::vec3 (5.0f, -2.5f, 10.0f)}
		);
	static_cube->set_parent (std::move (static_sphere).get ());

	scene->add_entity (std::move (static_plane));
	scene->add_entity (std::move (static_cube));
	scene->add_entity (std::move (static_sphere));

	engine->request_scene (std::move (scene));
	engine->run ();

	return 0;
}

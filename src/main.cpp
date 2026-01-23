#include "engine/engine.h"
#include "object/demos/instancing.h"
#include "scene.h"

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();

	std::unique_ptr<Scene> scene = std::make_unique<Scene> ();
	scene->add_object (std::make_unique<InstancingDemo> ());

	engine->request_scene (std::move (scene));
	engine->run ();

	return 0;
}

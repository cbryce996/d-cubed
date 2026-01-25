
#include "engine.h"
#include "entity/demos/wave.h"
#include "scene.h"

#include <memory>

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();

	std::unique_ptr<Scene> scene = std::make_unique<Scene> ();
	scene->add_entity (std::make_unique<Wave> ());

	engine->request_scene (std::move (scene));
	engine->run ();

	return 0;
}

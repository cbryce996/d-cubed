#include "engine/engine.h"
#include "simulation/demos/instancing.h"

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();
	std::unique_ptr<ISimulation> simulation
		= std::make_unique<InstancingDemo> ();
	engine->request_simulation (std::move (simulation));
	engine->run ();

	return 0;
}

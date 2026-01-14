#include "engine/engine.h"
#include "simulation.h"

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();
	Simulation* simulation = new Simulation (std::move (engine));
	simulation->run ();

	return 0;
}

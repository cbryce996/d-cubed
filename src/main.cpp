#include "engine/engine.h"
#include "simulation/simulation.h"

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();
	Game* game = new Game (std::move (engine));
	game->run ();

	return 0;
}

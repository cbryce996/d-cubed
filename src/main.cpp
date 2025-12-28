#include "engine/engine.h"
#include "game/game.h"

int main () {
	std::unique_ptr<Engine> engine = std::make_unique<Engine> ();
	Game* game = new Game (std::move (engine));
	game->run ();

	return 0;
}

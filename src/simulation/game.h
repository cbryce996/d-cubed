#ifndef GAME_H
#define GAME_H

#include "engine/engine.h"

class Game {
  public:
	std::mutex mutex;

	explicit Game (std::unique_ptr<Engine> engine);
	~Game ();

	void run ();
	void write_game_state (RenderState* render_state);

  private:
	std::unique_ptr<Engine> engine;
	bool running = false;
};

#endif // GAME_H

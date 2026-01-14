#ifndef GAME_H
#define GAME_H

#include "../engine/engine.h"

#include <mutex>

class Simulation {
  public:
	std::mutex mutex;

	explicit Simulation (std::unique_ptr<Engine> engine);
	~Simulation ();

	void run ();
	void write_game_state (RenderState* render_state);

  private:
	std::unique_ptr<Engine> engine;
	bool running = false;
};

#endif // GAME_H

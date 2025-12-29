#ifndef GAME_H
#define GAME_H

#include "engine/engine.h"
#include "engine/simulation/simulation.h"

class Game {
  public:
	std::mutex mutex;

	explicit Game (std::unique_ptr<Engine> engine);
	~Game ();

	void run ();
	static void handle_input (const InputManager& input);
	void setup_cubes ();
	void write_game_state (RenderState* render_state);

  private:
	std::unique_ptr<Engine> engine;

	bool running = false;

	float decay_timer = 0.0f;
	static constexpr float DECAY_INTERVAL_MS = 3000.0f;

	float crafting_timer = 0.0f;
	static constexpr float CRAFTING_INTERVAL_MS = 1000.0f;

	void calculate_item_decays (float delta_time_ms);
	void calculate_item_crafting_progress (float delta_time_ms);
};

#endif // GAME_H
